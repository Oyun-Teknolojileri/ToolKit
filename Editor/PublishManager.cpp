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

    PublishManager::~PublishManager() { SafeDel(m_webPublisher); }

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
      GetLogger()->WriteConsole(LogType::Error, "windows build not implemented");
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
        String buildLocation = ConcatPaths({projectLocation, "Android/app/build/outputs/apk/debug"});
        NormalizePath(buildLocation);
        GetLogger()->WriteConsole(LogType::Success, "Android build successfully finished.");
        GetLogger()->WriteConsole(LogType::Memo, "Exported APK location: %s", buildLocation.c_str());

        // open generated apk folder location. (windows only)
        std::system(("explorer /e, " + buildLocation).c_str());
      };

      g_app->m_statusMsg = "building android apk...";
      // use "gradlew bundle" command to build .aab project or use "gradlew assemble" to release build
      g_app->ExecSysCommand("gradlew assembleDebug", true, true, afterBuildFn);

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

      Path newWorkDir(ConcatPaths({"..", "Web"}));
      std::filesystem::current_path(newWorkDir);
      int toolKitCompileResult = std::system(ConcatPaths({"..", "Web", "Release.bat"}).c_str());
      if (toolKitCompileResult != 0)
      {
        exitWithErrorFn("ToolKit could not be compiled!");
        return;
      }
      newWorkDir = Path(ConcatPaths({ResourcePath(), "..", "Codes", "Web"}));
      if (!std::filesystem::exists(newWorkDir))
      {
        std::filesystem::create_directories(newWorkDir);
      }

      // Create build script
      const String pluginWebBuildScriptsFolder = ConcatPaths({ResourcePath(), "..", "Codes", "Web", "Release.bat"});
      std::ofstream releaseBuildScript(pluginWebBuildScriptsFolder.c_str());
      releaseBuildScript << "emcmake cmake -DEMSCRIPTEN=TRUE -DTK_CXX_EXTRA:STRING=\" -O3\" "
                            "-S .. -G Ninja && ninja & pause";
      releaseBuildScript.close();

      // Run scripts
      std::filesystem::current_path(newWorkDir);
      int pluginCompileResult = std::system(pluginWebBuildScriptsFolder.c_str());
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
      if (std::filesystem::exists(publishDirectory))
      {
        std::filesystem::remove_all(publishDirectory);
      }

      std::filesystem::create_directories(publishDirectory);
      for (int i = 0; i < ArraySize(files); i++)
      {
        std::filesystem::copy(files[i].c_str(), publishDirectory);
      }

      // Copy engine settings to config folder
      String configDirectory = ConcatPaths({ResourcePath(), "..", "Config"});
      if (!std::filesystem::exists(configDirectory))
      {
        std::filesystem::create_directories(configDirectory);
      }

      const String engineSettingsPath     = ConcatPaths({ConfigPath(), "Engine.settings"});
      const String destEngineSettingsPath = ConcatPaths({configDirectory, "Engine.settings"});

      if (!std::filesystem::exists(destEngineSettingsPath))
      {
        std::filesystem::copy(Path(engineSettingsPath.c_str()),
                              Path(destEngineSettingsPath.c_str()),
                              std::filesystem::copy_options::overwrite_existing);
      }

      // Create run script
      std::ofstream runBatchFile(ConcatPaths({publishDirectory, "Run.bat"}).c_str());
      runBatchFile << "emrun ./" + projectName + ".html";
      runBatchFile.close();

      // Output user about where are the output files
      GetLogger()->WriteConsole(LogType::Success, "Building for web has been completed successfully.");
      GetLogger()->WriteConsole(LogType::Memo, "Output files location: %s", publishDirectory);
    }

  } // namespace Editor
} // namespace ToolKit
