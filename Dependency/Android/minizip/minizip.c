#include <jni.h>
#include "unzip.h"
#include "zip.h"

#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>
#include <zlib.h>

#define MKDIR(d) mkdir(d, 0775)

const int WRITE_BUFFER_SIZE = 16384;
#define MAX_FILENAME_LEN 256

// Errors id
const int ERROR_CREATE_ZIP = -100;
const int ERROR_GET_CRC32 = -101;
const int ERROR_WHILE_READ = -102;
const int ERROR_FILE_NOT_FOUND = -103;
const int ERROR_ZIP_FILE_NOT_FOUND = -104;
const int ERROR_ZIP_FILE = -105;


void getFileTime(const char *filename, tm_zip *tmzip, uLong *dostime) {
    struct stat s = { 0 };
    time_t tm_t = 0;

    if (strcmp(filename, "-") != 0) {
        char name[MAX_FILENAME_LEN + 1];

        int len = strlen(filename);
        if (len > MAX_FILENAME_LEN) {
            len = MAX_FILENAME_LEN;
        }

        strncpy(name, filename, MAX_FILENAME_LEN - 1);
        name[MAX_FILENAME_LEN] = 0;

        if (name[len - 1] == '/') {
            name[len - 1] = 0;
        }

        /* not all systems allow stat'ing a file with / appended */
        if (stat(name, &s) == 0) {
            tm_t = s.st_mtime;
        }
    }

    struct tm* filedate = localtime(&tm_t);
    tmzip->tm_sec  = filedate->tm_sec;
    tmzip->tm_min  = filedate->tm_min;
    tmzip->tm_hour = filedate->tm_hour;
    tmzip->tm_mday = filedate->tm_mday;
    tmzip->tm_mon  = filedate->tm_mon;
    tmzip->tm_year = filedate->tm_year;
}

void setFileTime(const char *filename, uLong dosdate, tm_unz tmu_date) {
    struct tm newdate;
    newdate.tm_sec  = tmu_date.tm_sec;
    newdate.tm_min  = tmu_date.tm_min;
    newdate.tm_hour = tmu_date.tm_hour;
    newdate.tm_mday = tmu_date.tm_mday;
    newdate.tm_mon  = tmu_date.tm_mon;

    if (tmu_date.tm_year > 1900) {
        newdate.tm_year = tmu_date.tm_year - 1900;
    } else {
        newdate.tm_year = tmu_date.tm_year;
    }
    newdate.tm_isdst = -1;

    struct utimbuf ut;
    ut.actime = ut.modtime = mktime(&newdate);
    utime(filename, &ut);
}

int isLargeFile(const char* filename) {
    FILE* pFile = fopen64(filename, "rb");
    if (pFile == NULL) return 0;

    fseeko64(pFile, 0, SEEK_END);
    ZPOS64_T pos = ftello64(pFile);
    fclose(pFile);

    return (pos >= 0xffffffff);
}

// Calculate the CRC32 of a file
int getCRC32(const char* filenameinzip, Bytef *buf, unsigned long size_buf, unsigned long* result_crc) {
    unsigned long calculate_crc = 0;

    int status = ZIP_OK;

    FILE *fin = fopen64(filenameinzip, "rb");
    if (fin == NULL) status = ERROR_GET_CRC32;
    else {
        unsigned long size_read = 0;
        do {
            size_read = (int) fread(buf, 1, size_buf, fin);

            if ((size_read < size_buf) && (feof(fin) == 0)) {
                status = ERROR_WHILE_READ;
            }

            if (size_read > 0) {
                calculate_crc = crc32(calculate_crc, buf, size_read);
            }
        } while ((status == ZIP_OK) && (size_read > 0));
    }

    if (fin) {
        fclose(fin);
    }

    *result_crc = calculate_crc;
    return status;
}

int extractCurrentFile(unzFile uf, const char *password) {
    unz_file_info64 file_info = { 0 };
    char filename_inzip[MAX_FILENAME_LEN] = { 0 };

    int status = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
    if (status != UNZ_OK) {
    	return status;
    }

    uInt size_buf = WRITE_BUFFER_SIZE;
    void* buf = (void*) malloc(size_buf);
    if (buf == NULL) return UNZ_INTERNALERROR;

    status = unzOpenCurrentFilePassword(uf, password);
    const char* write_filename = filename_inzip;

    // Create the file on disk so we can unzip to it
    FILE* fout = NULL;
    if (status == UNZ_OK) {
        fout = fopen64(write_filename, "wb");
    }

    // Read from the zip, unzip to buffer, and write to disk
    if (fout != NULL) {
        do {
            status = unzReadCurrentFile(uf, buf, size_buf);
            if (status <= 0) break;
            if (fwrite(buf, status, 1, fout) != 1) {
                status = UNZ_ERRNO;
                break;
            }
        } while (status > 0);

        if (fout) fclose(fout);

        // Set the time of the file that has been unzipped
        if (status == 0) {
        	setFileTime(write_filename, file_info.dosDate, file_info.tmu_date);
        }
    }

    unzCloseCurrentFile(uf);

    free(buf);
    return status;
}

