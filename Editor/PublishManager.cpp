/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "PublishManager.h"
#include "FileManager.h"

#include "App.h"

#include "DebugNew.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

namespace ToolKit
{
  namespace Editor
  {
    PublishManager::PublishManager()
    {
      m_webPublisher     = new WebPublisher();
      m_windowsPublisher = new WindowsPublisher();
      m_androidPublisher = new AndroidPublisher();
    }

    PublishManager::~PublishManager()
    {
      SafeDel(m_webPublisher);
      SafeDel(m_windowsPublisher);
      SafeDel(m_androidPublisher);
    }

    void PublishManager::Publish(PublishPlatform platform)
    {
      g_app->PackResources();

      if (platform == PublishPlatform::Web)
      {
        m_webPublisher->Publish();
      }

      if (platform == PublishPlatform::Android)
      {
        m_androidPublisher->Publish();
      }

      if (platform == PublishPlatform::Windows)
      {
        m_windowsPublisher->Publish();
      }
    }

    void WindowsPublisher::Publish() const
    {
      Path workDir = std::filesystem::current_path();

      std::error_code ec;
      auto returnLoggingError = [&ec, &workDir](bool setPathBack = false) -> bool
      {
        bool ret     = false;
        bool setback = setPathBack;
        if (ec)
        {
          TK_ERR("%s", ec.message().c_str());
          setback = true;
          ret     = true;
        }

        if (setback)
        {
          std::filesystem::current_path(workDir, ec);
          if (ec)
          {
            TK_ERR("%s", ec.message().c_str());
            TK_ERR("%s", "******** PLEASE RESTART THE EDITOR ********");
          }

          ret = true;
        }

        return ret;
      };

      // Run toolkit compile script
      Path newWorkDir(ConcatPaths({"..", "BuildScripts"}));
      std::filesystem::current_path(newWorkDir);
      int toolKitCompileResult = g_app->ExecSysCommand("WinBuildRelease.bat", false, true);
      if (toolKitCompileResult != 0)
      {
        returnLoggingError(true);
        TK_ERR("ToolKit could not be compiled");
        return;
      }

      // Run plugin compile script
      newWorkDir = Path(ConcatPaths({ResourcePath(), "..", "Windows"}));
      std::filesystem::current_path(newWorkDir, ec);
      if (returnLoggingError())
      {
        return;
      }

      int pluginCompileResult = g_app->ExecSysCommand("WinBuildRelease.bat", false, true);
      if (pluginCompileResult != 0)
      {
        returnLoggingError(true);
        TK_ERR("Windows build has failed!");
        return;
      }
      std::filesystem::current_path(workDir, ec);
      if (returnLoggingError())
      {
        return;
      }

      // Move files to publish directory
      const String projectName      = g_app->m_workspace.GetActiveProject().name;
      const String publishDirectory = ConcatPaths({ResourcePath(), "..", "Publish", "Windows"});
      const String publishBinDir    = ConcatPaths({publishDirectory, "Bin"});
      const String publishConfigDir = ConcatPaths({publishDirectory, "Config"});

      const String exeFile =
          ConcatPaths({ResourcePath(), "..", "Codes", "Bin"}) + GetPathSeparatorAsStr() + projectName + ".exe";

      const String pakFile                = ConcatPaths({ResourcePath(), "..", "MinResources.pak"});
      const String sdlDllPath             = ConcatPaths({workDir.string(), "SDL2.dll"});
      const String configDirectory        = ConcatPaths({ResourcePath(), "..", "Config"});
      const String engineSettingsPath     = ConcatPaths({ConfigPath(), "Engine.settings"});
      const String destEngineSettingsPath = ConcatPaths({publishConfigDir, "Engine.settings"});

      // Create directories
      if (!std::filesystem::exists(publishDirectory))
      {
        std::filesystem::create_directories(publishDirectory, ec);
        if (returnLoggingError())
        {
          return;
        }
      }
      if (!std::filesystem::exists(publishBinDir))
      {
        std::filesystem::create_directories(publishBinDir, ec);
        if (returnLoggingError())
        {
          return;
        }
      }
      if (!std::filesystem::exists(publishConfigDir))
      {
        std::filesystem::create_directories(publishConfigDir, ec);
        if (returnLoggingError())
        {
          return;
        }
      }

      // Copy exe file
      std::filesystem::copy(exeFile.c_str(), publishBinDir, std::filesystem::copy_options::overwrite_existing, ec);
      if (returnLoggingError())
      {
        return;
      }

      // Copy SDL2.dll from ToolKit bin folder to publish bin folder
      std::filesystem::copy(sdlDllPath.c_str(), publishBinDir, std::filesystem::copy_options::overwrite_existing, ec);
      if (returnLoggingError())
      {
        return;
      }

      // Copy pak
      std::filesystem::copy(pakFile.c_str(), publishDirectory, std::filesystem::copy_options::overwrite_existing, ec);
      if (returnLoggingError())
      {
        return;
      }

      // Copy engine settings to config folder
      std::filesystem::copy(engineSettingsPath.c_str(),
                            destEngineSettingsPath.c_str(),
                            std::filesystem::copy_options::overwrite_existing,
                            ec);
      if (returnLoggingError())
      {
        return;
      }

      // Tell user about where the location of output files is
      GetLogger()->WriteConsole(LogType::Success, "Building for WINDOWS has been completed successfully.");
      GetLogger()->WriteConsole(LogType::Memo,
                                "Output files location: %s",
                                std::filesystem::absolute(publishDirectory).string().c_str());

      g_app->m_shellOpenDirFn(publishDirectory);
    }

    void AndroidPublisher::PrepareIcon() const
    {
      String assetsPath = "Android/app/src/main/res";
      NormalizePath(assetsPath);
      
      String projectName = g_app->m_workspace.GetActiveProject().name;
      String resLocation = ConcatPaths({g_app->m_workspace.GetActiveWorkspace(), projectName, assetsPath});
      if (m_icon == nullptr) 
      {
        return;
      }

      int refWidth, refHeight, refComp;
      stbi_uc* refImage = stbi_load(m_icon->GetFile().c_str(), &refWidth, &refHeight, &refComp, 0);

      // search each folder in res folder and find icons, replace that icons with new one
      for (const auto& entry : std::filesystem::directory_iterator(resLocation))
      {
        if (!entry.is_directory())
        {
          continue;
        }
        
        for (auto& file : std::filesystem::directory_iterator(entry))
        {
          String name;
          String extension;
          String path = file.path().string();
          DecomposePath(path, nullptr, &name, &extension);
          // if this is image replace with new one, don't touch if this is background image
          if (name.find("background") == std::string::npos && extension == ".png")
          {
            int width, height, comp;
            // get the image that we want to replace
            stbi_uc* img = stbi_load(path.c_str(), &width, &height, &comp, 0);
            assert(img && "cannot load android icon");
            int res;
            res = stbir_resize_uint8(refImage, refWidth, refHeight, 0, img, width, height, 0, comp);
            assert(res && "cannot resize android icon");
            // write resized image
            res = stbi_write_png(path.c_str(), width, height, comp, img, 0);
            assert(res && "cannot write to android icon");
            stbi_image_free(img);
          }
        }
      }
      stbi_image_free(refImage);
    }

    void AndroidPublisher::EditAndroidManifest() const
    {
      String mainPath = "Android/app/src/main";
      NormalizePath(mainPath);
      
      String projectName = g_app->m_workspace.GetActiveProject().name;
      String mainLocation = ConcatPaths({g_app->m_workspace.GetActiveWorkspace(), projectName, mainPath}); 
      String manifestLoc = ConcatPaths({mainLocation, "AndroidManifest.xml"});
      String androidManifest = GetFileManager()->ReadAllText(ConcatPaths({mainLocation, "AndroidManifest.xml"}));
    
      String applicationName = m_appName.empty() ? projectName : m_appName;
      ReplaceFirstStringInPlace(androidManifest, "@string/app_name", applicationName);
      ReplaceFirstStringInPlace(androidManifest, "minSdkVersion=\"26\"", "minSdkVersion=\"" + std::to_string(m_minSdk) + "\"");
      ReplaceFirstStringInPlace(androidManifest, "maxSdkVersion=\"33\"", "maxSdkVersion=\"" + std::to_string(m_maxSdk) + "\"");
      GetFileManager()->WriteAllText(manifestLoc, androidManifest);
    }

    void AndroidPublisher::Publish() const
    {
      Path workDir = std::filesystem::current_path();

      std::error_code ec;
      auto returnLoggingError = [&ec, &workDir](bool setPathBack = false) -> bool
      {
        bool ret     = false;
        bool setback = setPathBack;
        if (ec)
        {
          TK_ERR("%s", ec.message().c_str());
          setback = true;
          ret     = true;
        }

        if (setback)
        {
          std::filesystem::current_path(workDir, ec);
          if (ec)
          {
            TK_ERR("%s", ec.message().c_str());
            TK_ERR("%s", "******** PLEASE RESTART THE EDITOR ********");
          }

          ret = true;
        }

        return ret;
      };

      String projectName = g_app->m_workspace.GetActiveProject().name;

      if (projectName.empty())
      {
        GetLogger()->WriteConsole(LogType::Error, "No project is loaded!");
        return;
      }

      String assetsPath = "Android/app/src/main/assets";
      NormalizePath(assetsPath);

      String projectLocation      = ConcatPaths({g_app->m_workspace.GetActiveWorkspace(), projectName});
      String sceneResourcesPath   = ConcatPaths({projectLocation, "MinResources.pak"});
      String androidResourcesPath = ConcatPaths({projectLocation, assetsPath, "MinResources.pak"});

      const std::filesystem::copy_options copyOption = std::filesystem::copy_options::overwrite_existing;

      std::filesystem::copy(sceneResourcesPath, androidResourcesPath, copyOption, ec);
      if (returnLoggingError())
      {
        return;
      }

      std::filesystem::current_path(ConcatPaths({projectLocation, "Android"}), ec);
      if (returnLoggingError())
      {
        return;
      }

      PrepareIcon();
      EditAndroidManifest();

      const auto afterBuildFn = [&](int res) -> void
      {
        if (res == 1)
        {
          GetLogger()->WriteConsole(LogType::Error, "Android build failed.");
          return;
        }
        String buildLocation = ConcatPaths({projectLocation, "Android/app/build/outputs/apk/release"});
        NormalizePath(buildLocation);
        const String publishDirStr  = ConcatPaths({ResourcePath(), "..", "Publish", "Android"});
        const String apkPathStr     = ConcatPaths({buildLocation, "app-release-unsigned.apk"});
        projectName                 = !m_appName.empty() ? m_appName : projectName;
        const String publishApkPath = ConcatPaths({publishDirStr, projectName + "_release.apk"});

        // Create directories
        if (!std::filesystem::exists(publishDirStr))
        {
          bool res = std::filesystem::create_directories(publishDirStr, ec);
          if (returnLoggingError())
          {
            return;
          }
        }

        std::filesystem::copy(apkPathStr.c_str(),
                              publishApkPath,
                              std::filesystem::copy_options::overwrite_existing,
                              ec);
        if (returnLoggingError())
        {
          return;
        }

        // Tell user about where the location of output files is
        GetLogger()->WriteConsole(LogType::Success, "Building for ANDROID has been completed successfully.");
        GetLogger()->WriteConsole(LogType::Memo,
                                  "Output files location: %s",
                                  std::filesystem::absolute(publishDirStr).string().c_str());

        g_app->m_shellOpenDirFn(publishDirStr);
      };

      g_app->m_statusMsg = "building android apk...";

      // use "gradlew bundle" command to build .aab project or use "gradlew assemble" to release build
      int compileResult = g_app->ExecSysCommand("gradlew assemble", false, true, afterBuildFn);
      if (compileResult != 0)
      {
        returnLoggingError(true);
        TK_ERR("Compiling failed.");
        return;
      }

      std::filesystem::current_path(workDir, ec); // set work directory back
      if (returnLoggingError())
      {
        return;
      }
    }

    void WebPublisher::Publish() const
    {
      Path workDir = std::filesystem::current_path();

      std::error_code ec;
      auto returnLoggingError = [&ec, &workDir](bool setPathBack = false) -> bool
      {
        bool ret     = false;
        bool setback = setPathBack;
        if (ec)
        {
          TK_ERR("%s", ec.message().c_str());
          setback = true;
          ret     = true;
        }

        if (setback)
        {
          std::filesystem::current_path(workDir, ec);
          if (ec)
          {
            TK_ERR("%s", ec.message().c_str());
            TK_ERR("%s", "******** PLEASE RESTART THE EDITOR ********");
          }

          ret = true;
        }

        return ret;
      };

      // Run toolkit compile script
      Path newWorkDir(ConcatPaths({"..", "BuildScripts"}));
      std::filesystem::current_path(newWorkDir, ec);
      if (returnLoggingError())
      {
        return;
      }

      int toolKitCompileResult = g_app->ExecSysCommand("WebBuildRelease.bat", false, true);
      if (toolKitCompileResult != 0)
      {
        returnLoggingError(true);
        TK_ERR("ToolKit could not be compiled!");
        return;
      }
      newWorkDir                               = Path(ConcatPaths({ResourcePath(), "..", "Web"}));

      const String pluginWebBuildScriptsFolder = ConcatPaths({ResourcePath(), "..", "Web", "WebBuildRelease.bat"});

      // Run scripts
      std::filesystem::current_path(newWorkDir, ec);
      if (returnLoggingError())
      {
        return;
      }

      int pluginCompileResult = g_app->ExecSysCommand(pluginWebBuildScriptsFolder.c_str(), false, true);
      if (pluginCompileResult != 0)
      {
        returnLoggingError(true);
        TK_ERR("Web build has failed!");
        return;
      }
      std::filesystem::current_path(workDir, ec);
      if (returnLoggingError())
      {
        return;
      }

      // Move files to a directory
      String projectName           = g_app->m_workspace.GetActiveProject().name;
      String publishDirectoryStr   = ConcatPaths({ResourcePath(), "..", "Publish", "Web"});
      const char* publishDirectory = publishDirectoryStr.c_str();
      String firstPart =
          ConcatPaths({ResourcePath(), "..", "Codes", "Bin"}) + GetPathSeparatorAsStr() + projectName + ".";
      String files[] = {firstPart + "data", firstPart + "html", firstPart + "js", firstPart + "wasm"};

      std::filesystem::create_directories(publishDirectory, ec);
      if (returnLoggingError())
      {
        return;
      }

      for (int i = 0; i < ArraySize(files); i++)
      {
        std::filesystem::copy(files[i].c_str(),
                              publishDirectory,
                              std::filesystem::copy_options::overwrite_existing,
                              ec);
        if (returnLoggingError())
        {
          return;
        }
      }

      // Create run script
      std::ofstream runBatchFile(ConcatPaths({publishDirectory, "Run.bat"}).c_str());
      runBatchFile << "emrun ./" + projectName + ".html";
      runBatchFile.close();

      std::filesystem::current_path(workDir, ec);
      if (returnLoggingError())
      {
        return;
      }

      // Output user about where are the output files
      GetLogger()->WriteConsole(LogType::Success, "Building for web has been completed successfully.");
      GetLogger()->WriteConsole(LogType::Memo,
                                "Output files location: %s",
                                std::filesystem::absolute(publishDirectory).string().c_str());

      g_app->m_shellOpenDirFn(publishDirectory);
    }

  } // namespace Editor
} // namespace ToolKit
