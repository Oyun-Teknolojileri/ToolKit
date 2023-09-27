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

#include "App.h"

#include "DebugNew.h"

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
      g_app->PackResources();

      Path workDir         = std::filesystem::current_path();

      auto exitWithErrorFn = [&workDir](const char* msg) -> void
      {
        GetLogger()->WriteConsole(LogType::Error, msg);
        std::filesystem::current_path(workDir);
      };

      // Run toolkit compile script
      Path newWorkDir(ConcatPaths({"..", "BuildScripts"}));
      std::filesystem::current_path(newWorkDir);
      int toolKitCompileResult = g_app->ExecSysCommand("WinBuildRelease.bat", false, true);
      if (toolKitCompileResult != 0)
      {
        exitWithErrorFn("ToolKit could not be compiled!");
        return;
      }

      // Run plugin compile script
      newWorkDir = Path(ConcatPaths({ResourcePath(), "..", "Windows"}));
      std::filesystem::current_path(newWorkDir);
      int pluginCompileResult = g_app->ExecSysCommand("WinBuildRelease.bat", false, true);
      if (pluginCompileResult != 0)
      {
        exitWithErrorFn("Plugin could not be compiled!");
        return;
      }
      std::filesystem::current_path(workDir);

      // Move files to publish directory
      const String projectName         = g_app->m_workspace.GetActiveProject().name;
      const String publishDirStr       = ConcatPaths({ResourcePath(), "..", "Publish", "Windows"});
      const String publishBinDirStr    = ConcatPaths({ResourcePath(), "..", "Publish", "Windows", "Bin"});
      const String publishConfigDirStr = ConcatPaths({ResourcePath(), "..", "Publish", "Windows", "Config"});
      const char* publishDirectory     = publishDirStr.c_str();
      const char* publishBinDir        = publishBinDirStr.c_str();
      const char* publishConfigDir     = publishConfigDirStr.c_str();

      const String exeFile =
          ConcatPaths({ResourcePath(), "..", "Codes", "Bin"}) + GetPathSeparatorAsStr() + projectName + ".exe";

      const String pakFile                = ConcatPaths({ResourcePath(), "..", "MinResources.pak"});
      const String destPakFilePath        = publishDirStr;
      const String sdlDllPath             = ConcatPaths({workDir.string(), "SDL2.dll"});
      const String configDirectory        = ConcatPaths({ResourcePath(), "..", "Config"});
      const String engineSettingsPath     = ConcatPaths({ConfigPath(), "Engine.settings"});
      const String destEngineSettingsPath = ConcatPaths({publishConfigDirStr, "Engine.settings"});

      // Create directories
      if (!std::filesystem::exists(publishDirectory))
      {
        if (!std::filesystem::create_directories(publishDirectory))
        {
          exitWithErrorFn("Could not create publish directory.");
        }
      }
      if (!std::filesystem::exists(publishBinDir))
      {
        if (!std::filesystem::create_directories(publishBinDir))
        {
          exitWithErrorFn("Could not create publish bin directory.");
        }
      }
      if (!std::filesystem::exists(publishConfigDir))
      {
        if (!std::filesystem::create_directories(publishConfigDir))
        {
          exitWithErrorFn("Could not create publish config directory.");
        }
      }

      // Copy exe file
      std::filesystem::copy(exeFile.c_str(), publishBinDir, std::filesystem::copy_options::overwrite_existing);

      // Copy SDL2.dll from ToolKit bin folder to publish bin folder
      std::filesystem::copy(sdlDllPath.c_str(), publishBinDir, std::filesystem::copy_options::overwrite_existing);

      // Copy pak
      std::filesystem::copy(pakFile.c_str(), destPakFilePath, std::filesystem::copy_options::overwrite_existing);

      // Copy engine settings to config folder
      std::filesystem::copy(engineSettingsPath.c_str(),
                            destEngineSettingsPath.c_str(),
                            std::filesystem::copy_options::overwrite_existing);

      // Tell user about where the location of output files is
      GetLogger()->WriteConsole(LogType::Success, "Building for WINDOWS has been completed successfully.");
      GetLogger()->WriteConsole(LogType::Memo,
                                "Output files location: %s",
                                std::filesystem::absolute(publishDirectory).string().c_str());

      g_app->m_shellOpenDirFn(publishDirectory);
    }

    void AndroidPublisher::Publish() const
    {
      g_app->PackResources();

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

      std::error_code ec;
      std::filesystem::copy(sceneResourcesPath, androidResourcesPath, copyOption, ec);
      if (ec)
      {
        TK_ERR("%s", ec.message().c_str());
        return;
      }

      Path workDir = std::filesystem::current_path(); // chace current work directory
      std::filesystem::current_path(ConcatPaths({projectLocation, "Android"}));

      const auto afterBuildFn = [projectName, projectLocation](int res) -> void
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
        const String publishApkPath = ConcatPaths({publishDirStr, projectName + "_release.apk"});

        // Create directories
        if (!std::filesystem::exists(publishDirStr))
        {
          bool res = std::filesystem::create_directories(publishDirStr);
          if (!res)
          {
            GetLogger()->WriteConsole(LogType::Error, "Could not create publish directory for android.");
            return;
          }
        }

        std::filesystem::copy(apkPathStr.c_str(), publishApkPath, std::filesystem::copy_options::overwrite_existing);

        // Tell user about where the location of output files is
        GetLogger()->WriteConsole(LogType::Success, "Building for ANDROID has been completed successfully.");
        GetLogger()->WriteConsole(LogType::Memo,
                                  "Output files location: %s",
                                  std::filesystem::absolute(publishDirStr).string().c_str());

        g_app->m_shellOpenDirFn(publishDirStr);
      };

      g_app->m_statusMsg = "building android apk...";

      // use "gradlew bundle" command to build .aab project or use "gradlew assemble" to release build
      g_app->ExecSysCommand("gradlew assemble", false, true, afterBuildFn);

      std::filesystem::current_path(workDir); // set work directory back
    }

    void WebPublisher::Publish() const
    {
      // Pak project
      g_app->PackResources();

      // Warning: Running batch files are Windows specific
      Path workDir         = std::filesystem::current_path();

      auto exitWithErrorFn = [&workDir](const char* msg) -> void
      {
        GetLogger()->WriteConsole(LogType::Error, msg);
        std::filesystem::current_path(workDir);
      };

      Path newWorkDir(ConcatPaths({"..", "BuildScripts"}));
      std::filesystem::current_path(newWorkDir);
      int toolKitCompileResult =
          g_app->ExecSysCommand(ConcatPaths({"..", "BuildScripts", "WebBuildRelease.bat"}).c_str(), false, true);
      if (toolKitCompileResult != 0)
      {
        exitWithErrorFn("ToolKit could not be compiled!");
        return;
      }
      newWorkDir                               = Path(ConcatPaths({ResourcePath(), "..", "Web"}));

      const String pluginWebBuildScriptsFolder = ConcatPaths({ResourcePath(), "..", "Web", "WebBuildRelease.bat"});

      // Run scripts
      std::filesystem::current_path(newWorkDir);
      int pluginCompileResult = g_app->ExecSysCommand(pluginWebBuildScriptsFolder.c_str(), false, true);
      if (pluginCompileResult != 0)
      {
        exitWithErrorFn("Plugin could not be compiled!");
        return;
      }
      std::filesystem::current_path(workDir);

      // Move files to a directory
      String projectName           = g_app->m_workspace.GetActiveProject().name;
      String publishDirectoryStr   = ConcatPaths({ResourcePath(), "..", "Publish", "Web"});
      const char* publishDirectory = publishDirectoryStr.c_str();
      String firstPart =
          ConcatPaths({ResourcePath(), "..", "Codes", "Bin"}) + GetPathSeparatorAsStr() + projectName + ".";
      String files[] = {firstPart + "data", firstPart + "html", firstPart + "js", firstPart + "wasm"};
      
      std::filesystem::create_directories(publishDirectory);
      for (int i = 0; i < ArraySize(files); i++)
      {
        std::filesystem::copy(files[i].c_str(), publishDirectory, std::filesystem::copy_options::overwrite_existing);
      }

      // Create run script
      std::ofstream runBatchFile(ConcatPaths({publishDirectory, "Run.bat"}).c_str());
      runBatchFile << "emrun ./" + projectName + ".html";
      runBatchFile.close();

      std::filesystem::current_path(workDir);

      // Output user about where are the output files
      GetLogger()->WriteConsole(LogType::Success, "Building for web has been completed successfully.");
      GetLogger()->WriteConsole(LogType::Memo,
                                "Output files location: %s",
                                std::filesystem::absolute(publishDirectory).string().c_str());

      g_app->m_shellOpenDirFn(publishDirectory);
    }

  } // namespace Editor
} // namespace ToolKit
