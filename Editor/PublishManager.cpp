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
      publishArguments        += m_appName.empty() ? g_app->m_workspace.GetActiveProject().name + '\n' : m_appName + '\n';
      publishArguments        += std::to_string((int) m_deployAfterBuild) + '\n';
      publishArguments        += std::to_string((int) m_isDebugBuild) + '\n';
      publishArguments        += std::to_string(m_minSdk) + '\n';
      publishArguments        += std::to_string(m_maxSdk) + '\n';
      publishArguments        += std::to_string(m_oriantation) + '\n';
      publishArguments        += std::to_string((int) platform) + '\n';
      publishArguments        += m_icon == nullptr ? "default" : m_icon->GetFile();

      GetFileManager()->WriteAllText("PublishArguments.txt", publishArguments);
      g_app->m_statusMsg = "Packing...";

      String packerPath  = NormalizePath("Utils/Packer/Packer.exe");
      // close zip file before running packer, because packer will use this file as well,
      // this will cause errors otherwise
      GetFileManager()->CloseZipFile();

      m_isBuilding = true;
      packerPath = std::filesystem::absolute(ConcatPaths({"..", packerPath})).string();

      const auto afterPackFn = [&](int res) -> void
      {
        if (std::filesystem::exists("PackerOutput.txt"))
        {
          TK_LOG(GetFileManager()->ReadAllText("PackerOutput.txt").c_str());
          std::filesystem::remove("PackerOutput.txt");
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
