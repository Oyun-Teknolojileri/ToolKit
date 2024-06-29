/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

#include <zlib.h>

#ifdef __ANDROID__
  #include <Android/minizip/unzip.h>
  #include <Android/minizip/zip.h>
#else
  #include <unzip.h>
  #include <zip.h>
#endif

namespace ToolKit
{
  class TK_API FileManager
  {
   public:
    ~FileManager();

    XmlFilePtr GetXmlFile(const String& filePath);
    uint8* GetImageFile(const String& filePath, int* x, int* y, int* comp, int reqComp);
    float* GetHdriFile(const String& filePath, int* x, int* y, int* comp, int reqComp);

    int PackResources(const String& path);
    void CloseZipFile();
    bool CheckFileFromResources(const String& path);
    void GetRelativeResourcesPath(String& path);
    bool CheckPakFile(); //!< Returns true if workspace contains pak file.
    String ReadAllText(const String& file);
    void WriteAllText(const String& file, const String& text);

   private:
    typedef std::variant<XmlFilePtr, uint8*, float*> FileDataType;

    enum class FileType
    {
      Xml,
      ImageUint8,
      ImageFloat
    };

    struct ImageFileInfo
    {
      String& filePath;
      int* x;
      int* y;
      int* comp;
      int reqComp;
    };

    FileDataType GetFile(FileType fileType, ImageFileInfo& fileInfo);
    void LoadAllScenes(const String& path);
    void GetAllUsedResourcePaths(const String& path);

    bool ZipPack(const String& zipName);
    bool AddFileToZip(zipFile zfile, const char* filename);

    void GetAllPaths(const String& path);
    void GetExtraFilePaths(const String& path);

    XmlFilePtr ReadXmlFileFromZip(zipFile zfile, const String& relativePath, const char* path);
    uint8* ReadImageFileFromZip(zipFile zfile, const String& relativePath, ImageFileInfo& fileInfo);
    float* ReadHdriFileFromZip(zipFile zfile, const String& relativePath, ImageFileInfo& fileInfo);
    XmlFilePtr CreateXmlFileFromZip(zipFile zfile, const String& filename, uint filesize);
    uint8* CreateImageFileFromZip(zipFile zfile, uint filesize, ImageFileInfo& fileInfo);
    float* CreateHdriFileFromZip(zipFile zfile, uint filesize, ImageFileInfo& fileInfo);

    void GenerateOffsetTableForPakFiles();
    bool IsFileInPak(const String& filename);

   private:
    StringSet m_allPaths;
    std::unordered_map<String, std::pair<ZPOS64_T, uint32_t>> m_zipFilesOffsetTable;
    bool m_offsetTableCreated = false;
    zipFile m_zfile           = nullptr;

   public:
    bool m_ignorePakFile = false;
  };
} // namespace ToolKit
