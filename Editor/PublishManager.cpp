
#include "PublishManager.h"

#include "App.h"
#include "ToolKit.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {
    PublishManager::PublishManager() { m_webPublisher = new WebPublisher(); }

    PublishManager::~PublishManager() { SafeDel(m_webPublisher); }

    void PublishManager::Publish(PublishPlatform platform)
    {
      if (platform == PublishPlatform::Web)
      {
        m_webPublisher->Publish();
      }
    }

    void WebPublisher::Publish() const
    {
      // Pak project
      g_app->PackResources();

      // Warning: Running batch files are Windows specific

      Path workDir = std::filesystem::current_path();
      Path newWorkDir(ConcatPaths({"..", "Web"}));
      std::filesystem::current_path(newWorkDir);
      std::system(ConcatPaths({"..", "Web", "Release.bat"}).c_str());
      newWorkDir = Path(ConcatPaths({ResourcePath(), "..", "Codes", "Web"}));
      if (!std::filesystem::exists(newWorkDir))
      {
        std::filesystem::create_directories(newWorkDir);
      }

      // Create build script
      const String pluginWebBuildScriptsFolder =
          ConcatPaths({ResourcePath(), "..", "Codes", "Web", "Release.bat"});
      std::ofstream releaseBuildScript(pluginWebBuildScriptsFolder.c_str());
      releaseBuildScript
          << "emcmake cmake -DEMSCRIPTEN=TRUE -DTK_CXX_EXTRA:STRING=\" -O3\" "
             "-S .. -G Ninja && ninja & pause";
      releaseBuildScript.close();

      // Run scripts
      std::filesystem::current_path(newWorkDir);
      std::system(pluginWebBuildScriptsFolder.c_str());
      std::filesystem::current_path(workDir);

      // Move files to a directory
      String projectName = g_app->m_workspace.GetActiveProject().name;
      String publishDirectoryStr =
          ConcatPaths({ResourcePath(), "..", "Publish", "Web"});
      const char* publishDirectory = publishDirectoryStr.c_str();
      String firstPart = ConcatPaths({ResourcePath(), "..", "Codes", "Bin"}) +
                         GetPathSeparatorAsStr() + projectName + ".";
      String files[] = {firstPart + "data",
                        firstPart + "html",
                        firstPart + "js",
                        firstPart + "wasm"};
      if (std::filesystem::exists(publishDirectory))
      {
        std::filesystem::remove_all(publishDirectory);
      }
      std::filesystem::create_directories(publishDirectory);
      for (int i = 0; i < 4; i++)
      {
        std::filesystem::copy(files[i].c_str(), publishDirectory);
      }

      // Copy engine settings to config folder
      String configDirectory = ConcatPaths({ResourcePath(), "..", "Config"});
      if (!std::filesystem::exists(configDirectory))
      {
        std::filesystem::create_directories(configDirectory);
      }

      const String engineSettingsPath =
          ConcatPaths({ConfigPath(), "Engine.settings"});
      const String destEngineSettingsPath =
          ConcatPaths({configDirectory, "Engine.settings"});

      if (!std::filesystem::exists(destEngineSettingsPath))
      {
        std::filesystem::copy(
            Path(engineSettingsPath.c_str()),
            Path(destEngineSettingsPath.c_str()),
            std::filesystem::copy_options::overwrite_existing);
      }

      // Create run script
      std::ofstream runBatchFile(
          ConcatPaths({publishDirectory, "Run.bat"}).c_str());
      runBatchFile << "emrun ./" + projectName + ".html";
      runBatchFile.close();

      // Output user about where are the output files
      GetLogger()->WriteConsole(
          LogType::Memo,
          "Building for web has been completed successfully.");
      GetLogger()->WriteConsole(LogType::Memo,
                                "Output files location: %s",
                                publishDirectory);
    }

  } // namespace Editor
} // namespace ToolKit
