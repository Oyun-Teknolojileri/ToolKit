
#include "FileManager.h"

#include <string>
#include <unordered_map>
#include <memory>

#include "ToolKit.h"

namespace ToolKit
{
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
      GetLogger()->WriteConsole(LogType::Warning, "Resources packed.");
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
        return false;
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

    fseek(f, 0, SEEK_END);
    flen = ftell(f);
    rewind(f);

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
      (flen > 0xffffffff) ? 1 : 0
    );
    if (ret != ZIP_OK)
    {
      fclose(f);
      zipCloseFileInZip(zfile);
      return false;
    }

    char* file_data = reinterpret_cast<char*>
    (
      malloc((flen + 1) * static_cast<unsigned int>(sizeof(char)))
    );
    red = fread(file_data, flen, 1, f);
    ret = zipWriteInFileInZip
    (
      zfile,
      file_data,
      static_cast<unsigned int>(red * flen)
    );
    if (ret != ZIP_OK)
    {
      fclose(f);
      zipCloseFileInZip(zfile);
      return false;
    }

    free(file_data);
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
}  // namespace ToolKit
