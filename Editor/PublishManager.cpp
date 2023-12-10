/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "PublishManager.h"

#include "App.h"
#include "FileManager.h"

#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <functional>
#include <thread>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {
    void PublishManager::Publish(PublishPlatform platform)
    {
      if (m_isBuilding)
      {
        TK_WRN("Toolkit already building an project");
        return;
      }

      String publishArguments  = g_app->m_workspace.GetActiveProject().name + '\n';
      publishArguments        += g_app->m_workspace.GetActiveWorkspace() + '\n';
      publishArguments += m_appName.empty() ? g_app->m_workspace.GetActiveProject().name + '\n' : m_appName + '\n';
      publishArguments += std::to_string((int) m_deployAfterBuild) + '\n';
      publishArguments += std::to_string((int) m_isDebugBuild) + '\n';
      publishArguments += std::to_string(m_minSdk) + '\n';
      publishArguments += std::to_string(m_maxSdk) + '\n';
      publishArguments += std::to_string(m_oriantation) + '\n';
      publishArguments += std::to_string((int) platform) + '\n';
      publishArguments += m_icon == nullptr ? TexturePath(ConcatPaths({"Icons", "app.png"}), true) : m_icon->GetFile();

      GetFileManager()->WriteAllText("PublishArguments.txt", publishArguments);
      g_app->m_statusMsg = "Packing...";

      String packerPath  = NormalizePath("Utils/Packer/Packer.exe");
      // close zip file before running packer, because packer will use this file as well,
      // this will cause errors otherwise
      GetFileManager()->CloseZipFile();

      m_isBuilding           = true;
      packerPath             = std::filesystem::absolute(ConcatPaths({"..", packerPath})).string();

      const auto afterPackFn = [&](int res) -> void
      {
        if (res != 1 && std::filesystem::exists("PackerOutput.txt"))
        {
          TK_LOG(GetFileManager()->ReadAllText("PackerOutput.txt").c_str());
        }
        TK_LOG("Build Ended.");
        m_isBuilding = false;
      };

      if (platform == PublishPlatform::Web)
      {
        TK_LOG("Packing to Web...");
        g_app->ExecSysCommand(packerPath, true, true, afterPackFn);
      }
      else if (platform == PublishPlatform::Android)
      {
        TK_LOG("Packing to Android...");
        g_app->ExecSysCommand(packerPath, true, true, afterPackFn);
      }
      else // windows build
      {
        TK_LOG("Packing to Windows...");
        std::thread thread = std::thread(RunPipe, packerPath, afterPackFn);
        thread.detach();
      }
    }
  } // namespace Editor
} // namespace ToolKit
