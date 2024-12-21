/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "FileManager.h"

#include "Audio.h"
#include "Logger.h"
#include "Material.h"
#include "Mesh.h"
#include "Scene.h"
#include "Shader.h"
#include "TKImage.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{
  FileManager::FileManager() {}

  FileManager::~FileManager()
  {
    if (m_zfile)
    {
      unzClose(m_zfile);
      m_zfile = nullptr;
    }
  }

  void FileManager::CloseZipFile()
  {
    if (m_zfile)
    {
      unzClose(m_zfile);
      m_zfile = nullptr;
    }
  }

  XmlFilePtr FileManager::GetXmlFile(const String& filePath)
  {
    String path            = filePath;
    ImageFileInfo fileInfo = {path, nullptr, nullptr, nullptr, 0};
    FileDataType data      = GetFile(FileType::Xml, fileInfo);
    return std::get<XmlFilePtr>(data);
  }

  uint8* FileManager::GetImageFile(const String& filePath, int* x, int* y, int* comp, int reqComp)
  {
    String path            = filePath;
    ImageFileInfo fileInfo = {path, x, y, comp, reqComp};
    FileDataType data      = GetFile(FileType::ImageUint8, fileInfo);
    return std::get<uint8*>(data);
  }

  float* FileManager::GetHdriFile(const String& filePath, int* x, int* y, int* comp, int reqComp)
  {
    String path            = filePath;
    ImageFileInfo fileInfo = {path, x, y, comp, reqComp};
    FileDataType data      = GetFile(FileType::ImageFloat, fileInfo);
    return std::get<float*>(data);
  }

  FileManager::FileDataType FileManager::GetFile(FileType fileType, ImageFileInfo& fileInfo)
  {
    String pakPath      = ConcatPaths({ResourcePath(), "..", "MinResources.pak"});

    // Get relative path from Resources directory
    String relativePath = fileInfo.filePath;
    GetRelativeResourcesPath(relativePath);

    if (!m_zfile)
    {
      m_zfile = unzOpen(pakPath.c_str());
    }

    if (m_zfile && !m_ignorePakFile)
    {
      GenerateOffsetTableForPakFiles();

      if (fileType == FileType::Xml)
      {
        return ReadXmlFileFromZip(m_zfile, relativePath, fileInfo.filePath.c_str());
      }
      else if (fileType == FileType::ImageUint8)
      {
        return ReadImageFileFromZip(m_zfile, relativePath, fileInfo);
      }
      else if (fileType == FileType::ImageFloat)
      {
        ImageSetVerticalOnLoad(true);
        float* img = ReadHdriFileFromZip(m_zfile, relativePath, fileInfo);
        ImageSetVerticalOnLoad(false);
        return img;
      }
      else if (fileType == FileType::Audio)
      {
        uint bufferSize   = 0;
        ubyte* fileBuffer = (ubyte*) ReadFileBufferFromZip(m_zfile, relativePath, bufferSize);
        if (bufferSize > 0)
        {
          if (AudioManager* audioMan = GetAudioManager())
          {
            return audioMan->DecodeFromMemory(fileBuffer, bufferSize);
          }
        }
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
        return MakeNewPtr<XmlFile>(fileInfo.filePath.c_str());
      }
      else if (fileType == FileType::ImageUint8)
      {
        return ImageLoad(fileInfo.filePath.c_str(), fileInfo.x, fileInfo.y, fileInfo.comp, fileInfo.reqComp);
      }
      else if (fileType == FileType::ImageFloat)
      {
        ImageSetVerticalOnLoad(true);
        float* img = ImageLoadF(fileInfo.filePath.c_str(), fileInfo.x, fileInfo.y, fileInfo.comp, fileInfo.reqComp);
        ImageSetVerticalOnLoad(false);
        return img;
      }
      else if (fileType == FileType::Audio)
      {
        if (AudioManager* audioMan = GetAudioManager())
        {
          return audioMan->DecodeFromFile(fileInfo.filePath);
        }
      }
      else
      {
        assert(false && "Unimplemented file type.");
      }
    }

    // Suppress warning.
    return (uint8*) nullptr;
  }

  SoundBuffer FileManager::GetAudioFile(const String& filePath)
  {
    String path            = filePath;
    ImageFileInfo fileInfo = {path, nullptr, nullptr, nullptr, 0};
    FileDataType data      = GetFile(FileType::Audio, fileInfo);

    return std::get<SoundBuffer>(data);
  }

  int FileManager::PackResources()
  {
    String zipFile = ConcatPaths({ResourcePath(), "..", "MinResources.pak"});

    if (CheckSystemFile(zipFile.c_str()))
    {
      if (m_zfile)
      {
        unzClose(m_zfile);
        m_zfile = nullptr;
      }

      std::error_code err;
      if (!std::filesystem::remove(zipFile, err))
      {
        TK_LOG("cannot remove MinResources.pak! message: %s\n", err.message().c_str());
        return -1;
      }
    }

    // Load all scenes once in order to fill resource managers
    TK_LOG("Packing Scenes\n");
    LoadAllScenes(ScenePath(""));

    TK_LOG("Packing Layers\n");
    LoadAllScenes(LayerPath(""));

    // Get all paths of resources
    TK_LOG("Getting all used paths\n");
    GetAllUsedResourcePaths();

    // Zip used resources
    if (!ZipPack(zipFile))
    {
      // Error
      TK_ERR("Error zipping.");
      return -1;
    }
    else
    {
      TK_LOG("Resources packed.\n");
    }

    // Copy assets under android assets if exists
    String androidAssetFolder = ConcatPaths({ResourcePath(), "..", "Android", "app", "src", "main", "assets"});
    String minResourcesPath   = ConcatPaths({ResourcePath(), "..", "MinResources.pak"});
    if (std::filesystem::exists(androidAssetFolder) && std::filesystem::exists(minResourcesPath))
    {
      std::error_code ec;
      std::filesystem::copy(minResourcesPath,
                            androidAssetFolder,
                            std::filesystem::copy_options::overwrite_existing,
                            ec);
    }

    return 0;
  }

  bool FileManager::CheckFileFromResources(const String& path)
  {
    if (!Main::GetInstance()->m_resourceRoot.empty())
    {
      GenerateOffsetTableForPakFiles();
    }

    String relativePath = path;
    UnixifyPath(relativePath);
    GetRelativeResourcesPath(relativePath);
    return CheckSystemFile(path) || IsFileInPak(relativePath);
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
      String name;
      DecomposePath(pt, nullptr, &name, nullptr);

      TK_LOG("Packing Scene: %s\n", name.c_str());

      ScenePtr scene = GetSceneManager()->Create<Scene>(pt);
      scene->Load();
      scene->Init();
    }
  }

  void FileManager::GetAllUsedResourcePaths()
  {
    std::unordered_map<String, ResourcePtr> mp;

    // Get all engine resources
    GetAllPaths(DefaultPath());

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
        absolutePath = ConcatPaths({DefaultAbsolutePath(), absolutePath});
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
        absolutePath = ConcatPaths({DefaultAbsolutePath(), absolutePath});
      }

      m_allPaths.insert(absolutePath);
    }

    // Animations
    mp = GetAnimationManager()->m_storage;
    for (auto it = mp.begin(); it != mp.end(); it++)
    {
      String absolutePath = it->first;
      // If the path is relative, make it absolute
      if (absolutePath[0] == '.')
      {
        size_t index = absolutePath.find("Meshes");
        absolutePath = absolutePath.substr(index);
        absolutePath = ConcatPaths({DefaultAbsolutePath(), absolutePath});
      }

      m_allPaths.insert(absolutePath);
    }

    // Skeletons
    mp = GetSkeletonManager()->m_storage;
    for (auto it = mp.begin(); it != mp.end(); it++)
    {
      String absolutePath = it->first;
      // If the path is relative, make it absolute
      if (absolutePath[0] == '.')
      {
        size_t index = absolutePath.find("Meshes");
        absolutePath = absolutePath.substr(index);
        absolutePath = ConcatPaths({DefaultAbsolutePath(), absolutePath});
      }

      m_allPaths.insert(absolutePath);
    }

    // Prefabs
    mp = GetSceneManager()->m_storage;
    for (auto it = mp.begin(); it != mp.end(); it++)
    {
      String absolutePath = it->first;
      // If the path is relative, make it absolute
      if (absolutePath[0] == '.')
      {
        size_t index = absolutePath.find("Prefabs");
        if (index == String::npos)
        {
          continue;
        }
        absolutePath = absolutePath.substr(index);
        absolutePath = ConcatPaths({DefaultAbsolutePath(), absolutePath});
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
        absolutePath = ConcatPaths({DefaultAbsolutePath(), absolutePath});
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
        absolutePath = ConcatPaths({DefaultAbsolutePath(), absolutePath});
      }

      m_allPaths.insert(absolutePath);
    }

    // Scenes
    GetAllPaths(ScenePath(""));

    // Layers
    GetAllPaths(LayerPath(""));

    // Extra files that the use provided
    GetExtraFilePaths();
  }

  bool FileManager::CheckPakFile() { return m_zfile != nullptr; }

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
    for (const String& path : m_allPaths)
    {
      if (!AddFileToZip(zFile, path.c_str()))
      {
        TK_WRN("Failed to add this file to zip: %s\n", path.c_str());
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
    stream.seekg(0, std::ios::end); // Determine stream size
    flen               = stream.tellg();

    String filenameStr = String(filename);
    size_t index       = filenameStr.find("Resources");
    if (index != String::npos)
    {
      constexpr int length = sizeof("Resources");
      filenameStr          = filenameStr.substr(index + length);
    }
    else
    {
      TK_ERR("Resource is not under resources path: %s", filename);
      return false;
    }

    // Compression level is -1 which is default, use 0 for no compression, 1 for best speed.
    ret =
        zipOpenNewFileInZip64(zfile, filenameStr.c_str(), NULL, NULL, 0, NULL, 0, NULL, MZ_COMPRESS_METHOD_ZSTD, -1, 0);

    if (ret != ZIP_OK)
    {
      fclose(f);
      zipCloseFileInZip(zfile);
      return false;
    }

    char* fileData = reinterpret_cast<char*>(malloc((flen + 1) * static_cast<uint>(sizeof(char))));
    red            = fread(fileData, 1, flen, f);
    ret            = zipWriteInFileInZip(zfile, fileData, static_cast<uint>(flen));

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

  void FileManager::GetAllPaths(const String& path)
  {
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
      if (entry.is_directory())
      {
        // Go animations in directories
        GetAllPaths(entry.path().string());
        continue;
      }

      m_allPaths.insert(std::filesystem::absolute(entry.path()).string());
    }
  }

  void FileManager::GetExtraFilePaths()
  {
    String extrFilesPathStr   = ConcatPaths({ResourcePath(), "ExtraFiles.txt"});
    const char* extrFilesPath = extrFilesPathStr.c_str();
    if (!CheckSystemFile(extrFilesPath))
    {
      GetLogger()->Log("'ExtraFiles.txt' is not found in resources path.");
      TK_WRN("'ExtraFiles.txt' is not found in resources path.");
      return;
    }

    // Open file and read
    String line;
    std::fstream file(extrFilesPath);
    while (std::getline(file, line))
    {
      line = Trim(line);
      if (line[0] == '#') // Comment
      {
        continue;
      }

      m_allPaths.insert(ConcatPaths({ResourcePath(), line}));
    }
    file.close();
  }

  String FileManager::ReadAllText(const String& file)
  {
    std::ifstream inputFile(file, std::ios::binary); // Replace with your file path

    if (!inputFile.is_open())
    {
      TK_ERR("cannot read all text! file: %s", file.c_str());
      assert(0);
      return "";
    }

    inputFile.seekg(0, std::ios::end);
    std::streampos fileSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    // Create a string of size, filled with spaces
    std::string fileContent(fileSize, ' ');
    inputFile.read(&fileContent[0], fileSize);
    return fileContent;
  }

  void FileManager::WriteAllText(const String& file, const String& text)
  {
    std::ofstream outputFile(file);

    if (!outputFile.is_open())
    {
      TK_ERR("cannot write all text! file: %s", file.c_str());
      assert(0);
      return;
    }

    outputFile << text; // Write the text to the file
    outputFile.close();
  }

  void FileManager::GetRelativeResourcesPath(String& path)
  {
    size_t index = path.find("Resources");
    if (index != String::npos)
    {
      constexpr int length = sizeof("Resources");
      path                 = path.substr(index + length);
    }
  }

  XmlFilePtr FileManager::ReadXmlFileFromZip(zipFile zfile, const String& relativePath, const char* path)
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

          if (unzGetCurrentFileInfo(zfile, &fileInfo, NULL, 0, NULL, 0, NULL, 0) == UNZ_OK)
          {
            XmlFilePtr file = CreateXmlFileFromZip(zfile, relativePath, static_cast<uint>(fileInfo.uncompressed_size));
            return file;
          }
        }
      }
    }

    // If the file is not found return the file from path
    return MakeNewPtr<XmlFile>(path);
  }

  uint8* FileManager::ReadImageFileFromZip(zipFile zfile, const String& relativePath, ImageFileInfo& fileInfo)
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
          unz_file_info unzFileInfo;
          memset(&unzFileInfo, 0, sizeof(unz_file_info));

          if (unzGetCurrentFileInfo(zfile, &unzFileInfo, NULL, 0, NULL, 0, NULL, 0) == UNZ_OK)
          {
            fileInfo.filePath = relativePath;
            uint8* img = CreateImageFileFromZip(zfile, static_cast<uint>(unzFileInfo.uncompressed_size), fileInfo);
            return img;
          }
        }
      }
    }

    // If the file is not found return the file from path
    return ImageLoad(fileInfo.filePath.c_str(), fileInfo.x, fileInfo.y, fileInfo.comp, fileInfo.reqComp);
  }

  float* FileManager::ReadHdriFileFromZip(zipFile zfile, const String& relativePath, ImageFileInfo& fileInfo)
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
          unz_file_info unzFileInfo;
          memset(&unzFileInfo, 0, sizeof(unz_file_info));

          if (unzGetCurrentFileInfo(zfile, &unzFileInfo, NULL, 0, NULL, 0, NULL, 0) == UNZ_OK)
          {
            fileInfo.filePath = relativePath;
            float* img = CreateHdriFileFromZip(zfile, static_cast<uint>(unzFileInfo.uncompressed_size), fileInfo);
            return img;
          }
        }
      }
    }

    // If the file is not found return the file from path
    return ImageLoadF(fileInfo.filePath.c_str(), fileInfo.x, fileInfo.y, fileInfo.comp, fileInfo.reqComp);
  }

  ubyte* FileManager::ReadFileBufferFromZip(zipFile zfile, const String& relativePath, uint& bufferSize)
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
          unz_file_info unzFileInfo;
          memset(&unzFileInfo, 0, sizeof(unz_file_info));

          if (unzGetCurrentFileInfo(zfile, &unzFileInfo, NULL, 0, NULL, 0, NULL, 0) == UNZ_OK)
          {
            // Read file
            bufferSize        = (uint64) unzFileInfo.uncompressed_size;
            ubyte* fileBuffer = new ubyte[bufferSize]();
            uint readBytes    = unzReadCurrentFile(zfile, fileBuffer, bufferSize);

            if (readBytes < 0)
            {
              GetLogger()->Log("Error reading compressed file: " + relativePath);
              SafeDelArray(fileBuffer);

              return nullptr;
            }
            else
            {
              return fileBuffer;
            }
          }
        }
      }
    }

    return nullptr;
  }

  XmlFilePtr FileManager::CreateXmlFileFromZip(zipFile zfile, const String& filename, uint filesize)
  {
    // Read file
    char* fileBuffer = new char[filesize]();
    uint readBytes   = unzReadCurrentFile(zfile, fileBuffer, filesize);
    if (readBytes < 0)
    {
      GetLogger()->Log("Error reading compressed file: " + filename);
      TK_ERR("Error reading compressed file: %s", filename.c_str());
    }

    // Create XmlFile object
    XmlFilePtr file = MakeNewPtr<XmlFile>(fileBuffer, readBytes);

    SafeDelArray(fileBuffer);

    return file;
  }

  uint8* FileManager::CreateImageFileFromZip(zipFile zfile, uint filesize, ImageFileInfo& fileInfo)
  {
    ubyte* fileBuffer = new ubyte[filesize]();
    uint readBytes    = unzReadCurrentFile(zfile, fileBuffer, filesize);
    if (readBytes < 0)
    {
      GetLogger()->Log("Error reading compressed file: " + fileInfo.filePath);
      TK_ERR("Error reading compressed file: %s", fileInfo.filePath.c_str());
    }

    // Load image
    uint8* img = ImageLoadFromMemory(fileBuffer, filesize, fileInfo.x, fileInfo.y, fileInfo.comp, fileInfo.reqComp);

    SafeDelArray(fileBuffer);

    return img;
  }

  float* FileManager::CreateHdriFileFromZip(zipFile zfile, uint filesize, ImageFileInfo& fileInfo)
  {
    ubyte* fileBuffer = new ubyte[filesize]();
    uint readBytes    = unzReadCurrentFile(zfile, fileBuffer, filesize);
    if (readBytes < 0)
    {
      GetLogger()->Log("Error reading compressed file: " + fileInfo.filePath);
      TK_ERR("Error reading compressed file: %s", fileInfo.filePath.c_str());
    }

    // Load image
    float* img = ImageLoadFromMemoryF(fileBuffer, filesize, fileInfo.x, fileInfo.y, fileInfo.comp, fileInfo.reqComp);

    SafeDelArray(fileBuffer);

    return img;
  }

  void FileManager::GenerateOffsetTableForPakFiles()
  {
    if (m_offsetTableCreated)
    {
      return;
    }

    String pakPath = ConcatPaths({ResourcePath(), "..", "MinResources.pak"});

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

          if (unzGetCurrentFileInfo(m_zfile, &fileInfo, NULL, 0, NULL, 0, NULL, 0) == UNZ_OK)
          {
            // Get file name
            char* filename = new char[fileInfo.size_filename + 1]();
            unzGetCurrentFileInfo(m_zfile, &fileInfo, filename, fileInfo.size_filename + 1, NULL, 0, NULL, 0);
            filename[fileInfo.size_filename] = '\0';

            std::pair<ZPOS64_T, uint> element;
            element.first  = unzGetOffset64(m_zfile);
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
    if (m_zipFilesOffsetTable.find(filename.c_str()) == m_zipFilesOffsetTable.end())
    {
      return false;
    }

    return true;
  }
} // namespace ToolKit
