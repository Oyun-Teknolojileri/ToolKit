
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

  XmlFilePtr FileManager::GetXmlFile(const String& filePath)
  {
    String path = filePath;
    FileInfo FileInfo = { path, nullptr, nullptr, nullptr, 0 };
    return std::get<XmlFilePtr>(GetFile(FileType::Xml, FileInfo));
  }

  uint8* FileManager::GetImageFile
  (
    const String& filePath,
    int* x,
    int* y,
    int* comp,
    int reqComp
  )
  {
    String path = filePath;
    FileInfo FileInfo = { path, x, y, comp, reqComp };
    return std::get<uint8*>(GetFile(FileType::ImageUint8, FileInfo));
  }

  FileManager::FileDataType FileManager::GetFile
  (
    FileType fileType,
    FileInfo& FileInfo
  )
  {
    String pakPath = ConcatPaths({ ResourcePath(), "..", "MinResources.pak" });

    // Get relative path from Resources directory
    String relativePath = FileInfo.filePath;
    GetRelativeResourcesPath(relativePath);

    if (!m_zfile)
    {
      m_zfile = unzOpen(pakPath.c_str());
    }

    if (m_zfile)
    {
      GenerateOffsetTableForPakFiles();

      if (fileType == FileType::Xml)
      {
        return ReadXmlFileFromZip
        (
          m_zfile,
          relativePath,
          FileInfo.filePath.c_str()
        );
      }
      else if (fileType == FileType::ImageUint8)
      {
        return ReadImageFileFromZip
        (
          m_zfile,
          relativePath,
          FileInfo
        );
      }
      else
      {
        assert(false && "Unimplemented file type.");
      }
    }
    else
    {
      // Zip pak not found, read from file at default path
      if (fileType == FileType::Xml)
      {
        return std::make_shared<XmlFile>(FileInfo.filePath.c_str());
      }
      else if (fileType == FileType::ImageUint8)
      {
        return stbi_load
        (
          FileInfo.filePath.c_str(),
          FileInfo.x,
          FileInfo.y,
          FileInfo.comp,
          FileInfo.reqComp
        );
      }
      else
      {
        assert(false && "Unimplemented file type.");
      }
    }

    // Supress warning
    uint8* temp = nullptr;
    return temp;
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

  XmlFilePtr FileManager::ReadXmlFileFromZip
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
            XmlFilePtr file = CreateXmlFileFromZip
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

    // If the file is not found return the file from path
    return std::make_shared<XmlFile>(path);
  }

  uint8* FileManager::ReadImageFileFromZip
  (
    zipFile zfile,
    const String& relativePath,
    FileInfo& FileInfo
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
            FileInfo.filePath = relativePath;
            uint8* img = CreateImageFileFromZip
            (
              zfile,
              static_cast<uint>(fileInfo.uncompressed_size),
              FileInfo
            );
            return img;
          }
        }
      }
    }

    // If the file is not found return the file from path
    return stbi_load
    (
      FileInfo.filePath.c_str(),
      FileInfo.x,
      FileInfo.y,
      FileInfo.comp,
      FileInfo.reqComp
    );
  }

  XmlFilePtr FileManager::CreateXmlFileFromZip
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
    XmlFilePtr file = std::make_shared<XmlFile>(fileBuffer, readBytes);

    SafeDelArray(fileBuffer);

    return file;
  }

  uint8* FileManager::CreateImageFileFromZip
  (
    zipFile zfile,
    uint filesize,
    FileInfo& fileSetings
  )
  {
    unsigned char* fileBuffer = new unsigned char[filesize]();
    uint readBytes = unzReadCurrentFile(zfile, fileBuffer, filesize);
    if (readBytes < 0)
    {
      GetLogger()->Log
      (
        "Error reading compressed file: " + fileSetings.filePath
      );
      GetLogger()->WriteConsole
      (
        LogType::Error,
        "Error reading compressed file: %s",
        fileSetings.filePath.c_str()
      );
    }

    // Load image
    uint8* img = stbi_load_from_memory
    (
      fileBuffer,
      filesize,
      fileSetings.x,
      fileSetings.y,
      fileSetings.comp,
      fileSetings.reqComp
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
