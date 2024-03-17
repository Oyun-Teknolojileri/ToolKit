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
    void PublishManager::Publish(PublishPlatform platform, PublishConfig publishConfig)
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
      publishArguments += std::to_string(m_minSdk) + '\n';
      publishArguments += std::to_string(m_maxSdk) + '\n';
      publishArguments += std::to_string(m_oriantation) + '\n';
      publishArguments += std::to_string((int) platform) + '\n';
      publishArguments += m_icon == nullptr ? TexturePath(ConcatPaths({"Icons", "app.png"}), true) : m_icon->GetFile();
      publishArguments += '\n';
      publishArguments += std::to_string((int) publishConfig) + '\n';

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
        if (res != 0)
        {
          TK_WRN("Build Failed");
        }
        else
        {
          TK_LOG("Build Ended.");
        }
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
        g_app->ExecSysCommand(packerPath, true, true, afterPackFn);
      }
    }
  } // namespace Editor
} // namespace ToolKit
