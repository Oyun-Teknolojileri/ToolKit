
#include "FileManager.h"

#include <string.h>
#include <fstream>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "ToolKit.h"

namespace ToolKit
{
  FileManager::~FileManager()
  {
    if (m_zfile)
    {
      unzClose(m_zfile);
    }
  }

  XmlFile FileManager::GetXmlFile(const String& path)
  {
    String pakPath = ConcatPaths({ ResourcePath(), "..", "MinResources.pak" });

    // Get relative path from Resources directory
    String relativePath = GetRelativeResourcesPath(path);
    if (relativePath.empty())
    {
      return nullptr;
    }
    const char* relativePathC = relativePath.c_str();

    if (!m_zfile)
    {
      m_zfile = unzOpen(pakPath.c_str());
    }

    if (m_zfile)
    {
      GenerateOffsetTableForPakFiles(m_zfile);
      XmlFile file = ReadXmlFileFromZip(m_zfile, relativePathC, path.c_str());

      return file;
    }
    else
    {
      // Zip pak not found, read from file at default path
      XmlFile file = XmlFile(path.c_str());

      return file;
    }
  }

  uint8* FileManager::GetImageFile
  (
    const String& path,
    int* x,
    int* y,
    int* comp,
    int reqComp
  )
  {
    String pakPath = ConcatPaths({ ResourcePath(), "..", "MinResources.pak" });

    // Get relative path from Resources directory
    String relativePath = GetRelativeResourcesPath(path);
    if (relativePath.empty())
    {
      return nullptr;
    }
    const char* relativePathC = relativePath.c_str();

    if (!m_zfile)
    {
      m_zfile = unzOpen(pakPath.c_str());
    }

    if (m_zfile)
    {
      GenerateOffsetTableForPakFiles(m_zfile);

      uint8* img = ReadImageFileFromZip
      (
        m_zfile,
        relativePathC,
        path.c_str(),
        x,
        y,
        comp,
        reqComp
      );

      return img;
    }
    else
    {
      // Zip pak not found
      uint8* img = stbi_load(path.c_str(), x, y, comp, reqComp);

      return img;
    }
  }

  void FileManager::PackResources(const String& path)
  {
    // Load all scenes once in order to fill resource managers
    LoadAllScenes(path);

    // Get all paths of resources
    GetAllUsedResourcePaths(path);

    // Zip used resources
    if (!ZipPack())
    {
      // Error
      GetLogger()->WriteConsole(LogType::Error, "Error zipping.");
    }
    else
    {
      GetLogger()->WriteConsole(LogType::Memo, "Resources packed.");
    }
  }

  void FileManager::LoadAllScenes(const String& path)
  {
    // Resources in the scene files
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
      if (entry.is_directory())
      {
        // Go scenes in directories
        LoadAllScenes(entry.path().string());
        continue;
      }

      // Load all scenes
      String pt = entry.path().string();
      ScenePtr scene = GetSceneManager()->Create<Scene>(pt);
      scene->Load();
      scene->Init();
    }
  }

  void FileManager::GetAllUsedResourcePaths(const String& path)
  {
    std::unordered_map<String, ResourcePtr> mp;

    // No manager for fonts

    // Material
    mp = GetMaterialManager()->m_storage;
    for (auto it = mp.begin(); it != mp.end(); it++)
    {
      String absolutePath = it->first;

      // Skip default.material
      if (!it->second->m_loaded)
      {
        continue;
      }

      // If the path is relative, make it absolute
      if (absolutePath[0] == '.')
      {
        size_t index = absolutePath.find("Materials");
        absolutePath = absolutePath.substr(index);
        absolutePath = ConcatPaths({ DefaultAbsolutePath(), absolutePath});
      }

      allPaths.insert(absolutePath);
    }

    // Meshes
    mp = GetMeshManager()->m_storage;
    for (auto it = mp.begin(); it != mp.end(); it++)
    {
      String absolutePath = it->first;
      // If the path is relative, make it absolute
      if (absolutePath[0] == '.')
      {
        size_t index = absolutePath.find("Meshes");
        absolutePath = absolutePath.substr(index);
        absolutePath = ConcatPaths({ DefaultAbsolutePath(), absolutePath });
      }

      allPaths.insert(absolutePath);
    }

    // Shaders
    mp = GetShaderManager()->m_storage;
    for (auto it = mp.begin(); it != mp.end(); it++)
    {
      String absolutePath = it->first;
      // If the path is relative, make it absolute
      if (absolutePath[0] == '.')
      {
        size_t index = absolutePath.find("Shaders");
        absolutePath = absolutePath.substr(index);
        absolutePath = ConcatPaths({ DefaultAbsolutePath(), absolutePath });
      }

      allPaths.insert(absolutePath);
    }

    // Textures
    mp = GetTextureManager()->m_storage;
    for (auto it = mp.begin(); it != mp.end(); it++)
    {
      String absolutePath = it->first;
      // If the path is relative, make it absolute
      if (absolutePath[0] == '.')
      {
        size_t index = absolutePath.find("Textures");
        absolutePath = absolutePath.substr(index);
        absolutePath = ConcatPaths({ DefaultAbsolutePath(), absolutePath });
      }

      allPaths.insert(absolutePath);
    }

    // Animations
    static String animPath = ConcatPaths({ ResourcePath(), "Meshes", "anim" });
    GetAnimationPaths(animPath);

    // Scenes
    GetScenePaths(path);
  }

  bool FileManager::ZipPack()
  {
    String zipName = ConcatPaths({ ResourcePath(), "..", "MinResources.pak" });
    zipFile zFile = zipOpen64(zipName.c_str(), 0);

    if (zFile == NULL)
    {
      printf("Error opening zip.\n");
      zipClose(zFile, NULL);
      return false;
    }

    // Add files to zip
    for (String path : allPaths)
    {
      if (!AddFileToZip(zFile, path.c_str()))
      {
        GetLogger()->WriteConsole(LogType::Error, "Error adding file to zip.");
      }
    }

    zipClose(zFile, NULL);

    return true;
  }

  bool FileManager::AddFileToZip(zipFile zfile, const char* filename)
  {
    FILE* f;
    int ret;
    size_t red;
    size_t flen;

    if (zfile == NULL || filename == NULL)
    {
      return false;
    }

    f = fopen(filename, "rb");
    if (f == NULL)
    {
      return false;
    }

    // Get length of the file
    std::basic_ifstream<char> stream(filename, std::ios::binary);
    if (!stream)
    {
      return false;
    }
    stream.unsetf(std::ios::skipws);
    stream.seekg(0, std::ios::end);  // Determine stream size
    flen = stream.tellg();

    String filenameStr = String(filename);
    size_t index = filenameStr.find("Resources");
    if (index != String::npos)
    {
      constexpr int length = sizeof("Resources");
      filenameStr = filenameStr.substr(index + length);
    }
    else
    {
      GetLogger()->WriteConsole
      (
        LogType::Error,
        "Resource is not under resources path: %s",
        filename
      );
      return false;
    }

    ret = zipOpenNewFileInZip64
    (
      zfile,
      filenameStr.c_str(),
      NULL,
      NULL,
      0,
      NULL,
      0,
      NULL,
      Z_DEFLATED,
      Z_DEFAULT_COMPRESSION,
      0
    );
    if (ret != ZIP_OK)
    {
      fclose(f);
      zipCloseFileInZip(zfile);
      return false;
    }

    char* fileData = reinterpret_cast<char*>
    (
      malloc((flen + 1) * static_cast<uint>(sizeof(char)))
    );
    red = fread(fileData, flen, 1, f);
    ret = zipWriteInFileInZip
    (
      zfile,
      fileData,
      static_cast<uint>(red * flen)
    );
    if (ret != ZIP_OK)
    {
      fclose(f);
      zipCloseFileInZip(zfile);
      return false;
    }

    free(fileData);
    fclose(f);
    zipCloseFileInZip(zfile);

    return true;
  }

  void FileManager::GetAnimationPaths(const String& path)
  {
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
      if (entry.is_directory())
      {
        // Go animations in directories
        GetAnimationPaths(entry.path().string());
        continue;
      }

      allPaths.insert(entry.path().string());
    }
  }

  void FileManager::GetScenePaths(const String& path)
  {
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
      if (entry.is_directory())
      {
        // Go scenes in directories
        GetScenePaths(entry.path().string());
        continue;
      }

      allPaths.insert(entry.path().string());
    }
  }

  String FileManager::GetRelativeResourcesPath(const String& path)
  {
    String filenameStr = String(path);
    size_t index = filenameStr.find("Resources");
    if (index != String::npos)
    {
      constexpr int length = sizeof("Resources");
      filenameStr = filenameStr.substr(index + length);
      return filenameStr;
    }
    else
    {
      GetLogger()->WriteConsole
      (
        LogType::Error,
        "Resource is not under resources path: %s",
        path
      );
      return "";
    }
  }

  XmlFile FileManager::ReadXmlFileFromZip
  (
    zipFile zfile,
    const char* relativePath,
    const char* path
  )
  {
    // Check offset map of file
    ZPOS64_T offset = m_zipFilesOffsetTable[relativePath];
    if (offset != 0)
    {
      if (unzSetOffset64(zfile, offset) == UNZ_OK)
      {
        if (unzOpenCurrentFile(zfile) == UNZ_OK)
        {
          unz_file_info fileInfo;
          memset(&fileInfo, 0, sizeof(unz_file_info));

          if
          (
            unzGetCurrentFileInfo
            (
              zfile,
              &fileInfo,
              NULL,
              0,
              NULL,
              0,
              NULL,
              0
            ) == UNZ_OK
          )
          {
            XmlFile file = CreateXmlFileFromZip
            (
              zfile,
              relativePath,
              static_cast<uint>(fileInfo.uncompressed_size)
            );
            return file;
          }
        }
      }
    }

    // If the file is not found in offset map, go over zip files
    if (unzGoToFirstFile(zfile) == UNZ_OK)
    {
      // Iterate over file info's
      do
      {
        if (unzOpenCurrentFile(zfile) == UNZ_OK)
        {
          unz_file_info fileInfo;
          memset(&fileInfo, 0, sizeof(unz_file_info));

          if
          (
            unzGetCurrentFileInfo
            (
              zfile,
              &fileInfo,
              NULL,
              0,
              NULL,
              0,
              NULL,
              0
            ) == UNZ_OK
          )
          {
            // Get file info
            char* filename = new char[fileInfo.size_filename + 1]();
            unzGetCurrentFileInfo
            (
              zfile,
              &fileInfo,
              filename,
              fileInfo.size_filename + 1,
              NULL,
              0,
              NULL,
              0
            );
            filename[fileInfo.size_filename] = '\0';

            if (strcmp(filename, relativePath))
            {
              delete[] filename;
              unzCloseCurrentFile(zfile);
              continue;
            }

            XmlFile file = CreateXmlFileFromZip
            (
              zfile,
              filename,
              static_cast<uint>(fileInfo.uncompressed_size)
            );

            delete[] filename;

            return file;
          }

          unzCloseCurrentFile(zfile);
        }
      } while (unzGoToNextFile(zfile) == UNZ_OK);
    }

    return XmlFile(path);
  }

  uint8* FileManager::ReadImageFileFromZip
  (
    zipFile zfile,
    const char* relativePath,
    const char* path,
    int* x,
    int* y,
    int* comp,
    int reqComp
  )
  {
    // Check offset map of file
    ZPOS64_T offset = m_zipFilesOffsetTable[relativePath];
    if (offset != 0)
    {
      if (unzSetOffset64(zfile, offset) == UNZ_OK)
      {
        if (unzOpenCurrentFile(zfile) == UNZ_OK)
        {
          unz_file_info fileInfo;
          memset(&fileInfo, 0, sizeof(unz_file_info));

          if
          (
            unzGetCurrentFileInfo
            (
              zfile,
              &fileInfo,
              NULL,
              0,
              NULL,
              0,
              NULL,
              0
            ) == UNZ_OK
          )
          {
            uint8* img = CreateImageFileFromZip
            (
              zfile,
              relativePath,
              static_cast<uint>(fileInfo.uncompressed_size),
              x,
              y,
              comp,
              reqComp
            );
            return img;
          }
        }
      }
    }

    // If the file is not found in offset map, go over zip files
    if (unzGoToFirstFile(zfile) == UNZ_OK)
    {
      // Iterate over file info's
      do
      {
        if (unzOpenCurrentFile(zfile) == UNZ_OK)
        {
          unz_file_info fileInfo;
          memset(&fileInfo, 0, sizeof(unz_file_info));

          if
          (
            unzGetCurrentFileInfo
            (
              zfile,
              &fileInfo,
              NULL,
              0,
              NULL,
              0,
              NULL,
              0
            ) == UNZ_OK
          )
          {
            // Get file info
            char* filename = new char[fileInfo.size_filename + 1]();
            unzGetCurrentFileInfo
            (
              zfile,
              &fileInfo,
              filename,
              fileInfo.size_filename + 1,
              NULL,
              0,
              NULL,
              0
            );
            filename[fileInfo.size_filename] = '\0';

            if (strcmp(filename, relativePath))
            {
              delete[] filename;
              unzCloseCurrentFile(zfile);
              continue;
            }

            // Read file
            uint8* img = CreateImageFileFromZip
            (
              zfile,
              filename,
              static_cast<uint>(fileInfo.uncompressed_size),
              x,
              y,
              comp,
              reqComp
            );

            delete[] filename;

            return img;
          }

          unzCloseCurrentFile(zfile);
        }
      } while (unzGoToNextFile(zfile) == UNZ_OK);
    }

    return stbi_load(path, x, y, comp, reqComp);
  }

  XmlFile FileManager::CreateXmlFileFromZip
  (
    zipFile zfile,
    const char* filename,
    uint filesize
  )
  {
    // Read file
    char* fileBuffer = new char[filesize]();
    uint readBytes = unzReadCurrentFile(zfile, fileBuffer, filesize);
    if (readBytes < 0)
    {
      GetLogger()->Log
      (
        "Error reading compressed file: " + String(filename)
      );
      GetLogger()->WriteConsole
      (
        LogType::Error,
        "Error reading compressed file: %s",
        filename
      );
    }

    // Create XmlFile object
    XmlFile file(fileBuffer, readBytes);

    delete[] fileBuffer;

    return file;
  }

  uint8* FileManager::CreateImageFileFromZip
  (
    zipFile zfile,
    const char* filename,
    uint filesize,
    int* x,
    int* y,
    int* comp,
    int reqComp
  )
  {
    unsigned char* fileBuffer = new unsigned char[filesize]();
    uint readBytes = unzReadCurrentFile(zfile, fileBuffer, filesize);
    if (readBytes < 0)
    {
      GetLogger()->Log
      (
        "Error reading compressed file: " + String(filename)
      );
      GetLogger()->WriteConsole
      (
        LogType::Error,
        "Error reading compressed file: %s",
        filename
      );
    }

    // Load image
    uint8* img = stbi_load_from_memory
    (
      fileBuffer,
      filesize,
      x,
      y,
      comp,
      reqComp
    );

    delete[] fileBuffer;

    return img;
  }

  void FileManager::GenerateOffsetTableForPakFiles(zipFile zfile)
  {
    if (m_offsetTableCreated)
    {
      return;
    }

    if (unzGoToFirstFile(zfile) == UNZ_OK)
    {
      do
      {
        if (unzOpenCurrentFile(zfile) == UNZ_OK)
        {
          unz_file_info fileInfo;
          memset(&fileInfo, 0, sizeof(unz_file_info));

          if
          (
            unzGetCurrentFileInfo
            (
              zfile,
              &fileInfo,
              NULL,
              0,
              NULL,
              0,
              NULL,
              0
            ) == UNZ_OK
          )
          {
            // Get file name
            char* filename = new char[fileInfo.size_filename + 1]();
            unzGetCurrentFileInfo
            (
              zfile,
              &fileInfo,
              filename,
              fileInfo.size_filename + 1,
              NULL,
              0,
              NULL,
              0
            );
            filename[fileInfo.size_filename] = '\0';

            ZPOS64_T offset = unzGetOffset64(zfile);

            m_zipFilesOffsetTable[filename] = offset;

            delete[] filename;
          }
        }
      } while (unzGoToNextFile(zfile) == UNZ_OK);
    }

    m_offsetTableCreated = true;
  }
}  // namespace ToolKit
