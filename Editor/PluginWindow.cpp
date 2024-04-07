/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "PluginWindow.h"

#include "Workspace.h"

#include <Plugin.h>
#include <PluginManager.h>

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

      ImGui::SetNextWindowSize(ImVec2(470, 110), ImGuiCond_Once);
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        if (ImGui::BeginTable("table1",
                              4,
                              ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersInnerH |
                                  ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterH |
                                  ImGuiTableFlags_BordersOuterV,
                              {0, 0}))
        {
          ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None, 0);
          ImGui::TableSetupColumn("Brief", ImGuiTableColumnFlags_None, 0);
          ImGui::TableSetupColumn("Load", ImGuiTableColumnFlags_None, 0);
          ImGui::TableSetupColumn("Compile", ImGuiTableColumnFlags_None, 0);
          ImGui::TableHeadersRow();
          ImGui::TableNextRow(0, 0);
          ImGui::TableSetColumnIndex(0);

          int imPluginId = 0;
          for (PluginSettings& plugin : m_plugins)
          {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::Text(plugin.name.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::AlignTextToFramePadding();
            ImGui::PushTextWrapPos(0);
            ImGui::TextUnformatted(plugin.brief.c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableSetColumnIndex(2);
            ImGui::AlignTextToFramePadding();

            ImGui::PushID(imPluginId++);

            PluginManager* plugMan = GetPluginManager();
            String fullPath        = plugin.file + GetPluginExtention();
            PluginRegister* reg    = plugMan->GetRegister(fullPath);
            bool isLoaded          = reg != nullptr && reg->m_loaded;

            if (ImGui::Checkbox("##Load", &isLoaded))
            {
              if (!isLoaded)
              {
                plugMan->Unload(fullPath);
              }
              else
              {
                plugMan->Load(plugin.file);
              }
            }
            UI::HelpMarker(TKLoc,
                           "Loads or unloads the plugin.\n"
                           "State is stored in engine settings and preserved on next editor run.\n"
                           "This may cause crash, save the work before.");
            ImGui::PopID();

            ImGui::TableSetColumnIndex(3);
            ImGui::AlignTextToFramePadding();
            if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_buildIcn), Vec2(20.0f)))
            {
              String pluginDir = g_app->m_workspace.GetPluginDirectory();
              String buildBat  = ConcatPaths({pluginDir, plugin.name, "Codes"});
              g_app->CompilePlugin(buildBat);
            }
            UI::HelpMarker(TKLoc,
                           "If a change is detected, compiles and reloads the plugin.\n"
                           "This may cause crash, save the work before.");
          }
        }

        ImGui::EndTable();
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