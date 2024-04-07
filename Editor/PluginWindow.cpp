/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "PluginWindow.h"

#include "Workspace.h"

#include <filesystem>

namespace ToolKit
{
  namespace Editor
  {

    TKDefineClass(PluginWindow, Window);

    PluginWindow::PluginWindow()
    {
      m_name = g_pluginWindow;
      ParsePluginDeclerations();
    }

    void PluginWindow::Show() 
    {
      ImGui::SetNextWindowSize(ImVec2(270, 110), ImGuiCond_Once);
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        for (const PluginSettings& plugin : m_plugins)
        {
          ImGui::Text(plugin.name.c_str());
          ImGui::Spacing();
        }
      }
      ImGui::End();

    }

    void PluginWindow::ParsePluginDeclerations()
    {
      m_plugins.clear();
      String plugDir = g_app->m_workspace.GetPluginDirectory();

      namespace fs   = std::filesystem;
      if (fs::exists(plugDir) && fs::is_directory(plugDir))
      {
        for (const auto& entry : fs::directory_iterator(plugDir))
        {
          String path    = entry.path().u8string();
          String cfgFile = ConcatPaths({path, "Config", "Plugin.settings"});
          if (CheckSystemFile(cfgFile))
          {
            PluginSettings pluginSet;
            pluginSet.Load(cfgFile);

            m_plugins.emplace_back(pluginSet);
          }
        }
      }
      else
      {
        TK_ERR("Can not traverse Plugins directory.");
      }
    }

  } // namespace Editor
} // namespace ToolKit