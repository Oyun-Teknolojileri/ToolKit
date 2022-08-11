
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

  XmlFile FileManager::GetXmlFile(const String& filePath)
  {
    String pakPath = ConcatPaths({ ResourcePath(), "..", "MinResources.pak" });

    // Get relative path from Resources directory
    String relativePath = filePath;
    GetRelativeResourcesPath(relativePath);

    if (!m_zfile)
    {
      m_zfile = unzOpen(pakPath.c_str());
    }

    if (m_zfile)
    {
      GenerateOffsetTableForPakFiles();

      XmlFile file = ReadXmlFileFromZip
      (
        m_zfile,
        relativePath,
        filePath.c_str()
      );

      return file;
    }
    else
    {
      // Zip pak not found, read from file at default path
      XmlFile file = XmlFile(filePath.c_str());

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
    String relativePath = path;
    GetRelativeResourcesPath(relativePath);
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
      GenerateOffsetTableForPakFiles();

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
    String zipName = ConcatPaths({ ResourcePath(), "..", "MinResources.pak" });
    if (std::filesystem::exists(zipName.c_str()))
    {
      if (m_zfile)
      {
        unzClose(m_zfile);
        m_zfile = nullptr;
      }
      std::filesystem::remove(zipName.c_str());
    }

    // Load all scenes once in order to fill resource managers
    LoadAllScenes(path);

    // Get all paths of resources
    GetAllUsedResourcePaths(path);

    // Zip used resources
    if (!ZipPack(zipName))
    {
      // Error
      GetLogger()->WriteConsole(LogType::Error, "Error zipping.");
    }
    else
    {
      GetLogger()->WriteConsole(LogType::Memo, "Resources packed.");
    }
  }

  bool FileManager::CheckFileFromResources(const String& path)
  {
    if (!Main::GetInstance()->m_resourceRoot.empty())
    {
      GenerateOffsetTableForPakFiles();
    }

    String relativePath = path;
    GetRelativeResourcesPath(relativePath);
    return std::filesystem::exists(path) || IsFileInPak(relativePath);
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

      m_allPaths.insert(absolutePath);
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

      m_allPaths.insert(absolutePath);
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

      m_allPaths.insert(absolutePath);
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

      m_allPaths.insert(absolutePath);
    }

    // Animations
    static String animPath = ConcatPaths({ ResourcePath(), "Meshes", "anim" });
    GetAnimationPaths(animPath);

    // Scenes
    GetScenePaths(path);

    // Extra files that the use provided
    GetExtraFilePaths(path);
  }

  bool FileManager::ZipPack(const String& zipName)
  {
    zipFile zFile = zipOpen64(zipName.c_str(), 0);

    if (zFile == NULL)
    {
      printf("Error opening zip.\n");
      zipClose(zFile, NULL);
      return false;
    }

    // Add files to zip
    for (String path : m_allPaths)
    {
      if (!AddFileToZip(zFile, path.c_str()))
      {
        GetLogger()->WriteConsole
        (
          LogType::Error,
          "Error adding file to zip. %s",
          path.c_str()
        );
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

      m_allPaths.insert(entry.path().string());
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

      m_allPaths.insert(entry.path().string());
    }
  }

  void FileManager::GetExtraFilePaths(const String& path)
  {
    String extrFilesPathStr = ConcatPaths({ path, "..", "ExtraFiles.txt" });
    const char* extrFilesPath = extrFilesPathStr.c_str();
    if (!std::filesystem::exists(extrFilesPath))
    {
      GetLogger()->Log("'ExtraFiles.txt' is not found in resources path.");
      GetLogger()->WriteConsole
      (
        LogType::Warning,
        "'ExtraFiles.txt' is not found in resources path."
      );
      return;
    }

    // Open file and read
    String line;
    std::fstream file(extrFilesPath);
    while (std::getline(file, line))
    {
      line = Trim(line);
      if (line[0] == '#')  // Comment
      {
        continue;
      }

      m_allPaths.insert(ConcatPaths({ ResourcePath(), line }));
    }

    file.close();
  }

  void FileManager::GetRelativeResourcesPath(String& path)
  {
    size_t index = path.find("Resources");
    if (index != String::npos)
    {
      constexpr int length = sizeof("Resources");
      path = path.substr(index + length);
    }
  }

  XmlFile FileManager::ReadXmlFileFromZip
  (
    zipFile zfile,
    const String& relativePath,
    const char* path
  )
  {
    // Check offset map of file
    String unixifiedPath = relativePath;
    UnixifyPath(unixifiedPath);
    ZPOS64_T offset = m_zipFilesOffsetTable[unixifiedPath].first;
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

            if (strcmp(filename, relativePath.c_str()))
            {
              SafeDelArray(filename);
              unzCloseCurrentFile(zfile);
              continue;
            }

            XmlFile file = CreateXmlFileFromZip
            (
              zfile,
              filename,
              static_cast<uint>(fileInfo.uncompressed_size)
            );

            SafeDelArray(filename);

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
    const String& relativePath,
    const char* path,
    int* x,
    int* y,
    int* comp,
    int reqComp
  )
  {
    // Check offset map of file
    String unixifiedPath = relativePath;
    UnixifyPath(unixifiedPath);
    ZPOS64_T offset = m_zipFilesOffsetTable[unixifiedPath].first;
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

            if (strcmp(filename, relativePath.c_str()))
            {
              SafeDelArray(filename);
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

            SafeDelArray(filename);

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
    const String& filename,
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
        "Error reading compressed file: " + filename
      );
      GetLogger()->WriteConsole
      (
        LogType::Error,
        "Error reading compressed file: %s",
        filename.c_str()
      );
    }

    // Create XmlFile object
    XmlFile file(fileBuffer, readBytes);

    SafeDelArray(fileBuffer);

    return file;
  }

  uint8* FileManager::CreateImageFileFromZip
  (
    zipFile zfile,
    const String& filename,
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
        "Error reading compressed file: " + filename
      );
      GetLogger()->WriteConsole
      (
        LogType::Error,
        "Error reading compressed file: %s",
        filename.c_str()
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

    SafeDelArray(fileBuffer);

    return img;
  }

  void FileManager::GenerateOffsetTableForPakFiles()
  {
    if (m_offsetTableCreated)
    {
      return;
    }

    String pakPath = ConcatPaths({ ResourcePath(), "..", "MinResources.pak" });

    if (!m_zfile)
    {
      m_zfile = unzOpen(pakPath.c_str());
    }

    if (unzGoToFirstFile(m_zfile) == UNZ_OK)
    {
      do
      {
        if (unzOpenCurrentFile(m_zfile) == UNZ_OK)
        {
          unz_file_info fileInfo;
          memset(&fileInfo, 0, sizeof(unz_file_info));

          if
          (
            unzGetCurrentFileInfo
            (
              m_zfile,
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
              m_zfile,
              &fileInfo,
              filename,
              fileInfo.size_filename + 1,
              NULL,
              0,
              NULL,
              0
            );
            filename[fileInfo.size_filename] = '\0';

            std::pair<ZPOS64_T, uint32_t> element;
            element.first = unzGetOffset64(m_zfile);
            element.second = fileInfo.uncompressed_size;

            // Unixfy the path
            String filenameStr(filename);
            UnixifyPath(filenameStr);

            m_zipFilesOffsetTable[filenameStr.c_str()] = element;

            SafeDelArray(filename);
          }
        }
      } while (unzGoToNextFile(m_zfile) == UNZ_OK);
    }

    m_offsetTableCreated = true;
  }

  bool FileManager::IsFileInPak(const String& filename)
  {
    if
    (
      m_zipFilesOffsetTable.find(filename.c_str())
      == m_zipFilesOffsetTable.end()
    )
    {
      return false;
    }

    return true;
  }
}  // namespace ToolKit
