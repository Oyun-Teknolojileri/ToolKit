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

    // PluginSettingsWindow
    //////////////////////////////////////////

    TKDefineClass(PluginSettingsWindow, Window);

    PluginSettingsWindow::PluginSettingsWindow() { m_name = "PluginSettings"; }

    void PluginSettingsWindow::Show()
    {
      ImGui::SetNextWindowSize(ImVec2(400, 375), ImGuiCond_Once);

      ImGui::Begin("Plugin Settings", &m_visible);

      // Editable fields for PluginSettings
      static char buffer[2048];

      // Plugin data
      ImGui::SeparatorText("Plugin");

      ImGui::BeginDisabled();
      strncpy(buffer, m_bckup.name.c_str(), IM_ARRAYSIZE(buffer));
      ImGui::InputText("Name", buffer, IM_ARRAYSIZE(buffer));
      m_bckup.name = buffer;
      ImGui::EndDisabled();

      strncpy(buffer, m_bckup.brief.c_str(), IM_ARRAYSIZE(buffer));
      ImGui::InputTextMultiline("Brief",
                                buffer,
                                IM_ARRAYSIZE(buffer),
                                ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4));
      m_bckup.brief = buffer;

      strncpy(buffer, m_bckup.version.c_str(), IM_ARRAYSIZE(buffer));
      ImGui::InputText("Version", buffer, IM_ARRAYSIZE(buffer));
      m_bckup.version = buffer;

      strncpy(buffer, m_bckup.engine.c_str(), IM_ARRAYSIZE(buffer));
      ImGui::InputText("Engine", buffer, IM_ARRAYSIZE(buffer));
      m_bckup.engine = buffer;

      ImGui::Checkbox("Auto Load", &m_bckup.autoLoad);

      // Developer data
      ImGui::SeparatorText("Developer");

      strncpy(buffer, m_bckup.developer.c_str(), IM_ARRAYSIZE(buffer));
      ImGui::InputText("Developer", buffer, IM_ARRAYSIZE(buffer));
      m_bckup.developer = buffer;

      strncpy(buffer, m_bckup.web.c_str(), IM_ARRAYSIZE(buffer));
      ImGui::InputText("Web", buffer, IM_ARRAYSIZE(buffer));
      m_bckup.web = buffer;

      strncpy(buffer, m_bckup.email.c_str(), IM_ARRAYSIZE(buffer));
      ImGui::InputText("Email", buffer, IM_ARRAYSIZE(buffer));
      m_bckup.email = buffer;

      ImGui::SeparatorText("File");

      if (ImGui::Button("Save"))
      {
        String file = PluginConfigPath(m_bckup.name);
        m_bckup.Save(file);
        *m_settings = m_bckup;
        RemoveFromUI();
      }

      ImGui::SameLine();

      if (ImGui::Button("Cancel"))
      {
        RemoveFromUI();
      }

      ImGui::End();
    }

    void PluginSettingsWindow::SetPluginSettings(PluginSettings* settings)
    {
      m_bckup    = *settings;
      m_settings = settings;
    }

    // PluginWindow
    //////////////////////////////////////////

    TKDefineClass(PluginWindow, Window);

    PluginWindow::PluginWindow()
    {
      m_name = g_pluginWindow;
      LoadPluginSettings();
    }

    void PluginWindow::Show()
    {
      ImGui::SetNextWindowSize(ImVec2(470, 110), ImGuiCond_Once);
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        if (ImGui::BeginTable("table1",
                              6,
                              ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersInnerH |
                                  ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterH |
                                  ImGuiTableFlags_BordersOuterV,
                              {0, 0}))
        {
          ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None, 0);
          ImGui::TableSetupColumn("Brief", ImGuiTableColumnFlags_None, 0);
          ImGui::TableSetupColumn("Load", ImGuiTableColumnFlags_None, 0);
          ImGui::TableSetupColumn("Compile", ImGuiTableColumnFlags_None, 0);
          ImGui::TableSetupColumn("Folder", ImGuiTableColumnFlags_None, 0);
          ImGui::TableSetupColumn("Settings", ImGuiTableColumnFlags_None, 0);
          ImGui::TableHeadersRow();
          ImGui::TableNextRow(0, 0);
          ImGui::TableSetColumnIndex(0);

          Vec2 btnSize   = Vec2(20.0f);
          int imPluginId = 0;
          for (PluginSettings& plugin : m_plugins)
          {
            ImGui::PushID(imPluginId++);

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
                reg                           = plugMan->Load(plugin.file);
                reg->m_plugin->m_currentState = PluginState::Running;
              }
            }
            UI::AddTooltipToLastItem("Loads or unloads the plugin.\n"
                                     "State is stored in engine settings and preserved on next editor run.\n"
                                     "This may cause crash, save the work before.");

            ImGui::TableSetColumnIndex(3);
            ImGui::AlignTextToFramePadding();
            if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_buildIcn), btnSize))
            {
              String pluginDir = g_app->m_workspace.GetPluginDirectory();
              String buildBat  = ConcatPaths({pluginDir, plugin.name, "Codes"});
              g_app->CompilePlugin(buildBat);
            }
            UI::AddTooltipToLastItem("If a change is detected, compiles and reloads the plugin.\n"
                                     "This may cause crash, save the work before.");

            ImGui::TableSetColumnIndex(4);
            ImGui::AlignTextToFramePadding();
            if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_folderIcon), btnSize))
            {
              String pluginDir = g_app->m_workspace.GetPluginDirectory();
              String dir       = ConcatPaths({pluginDir, plugin.name, "Codes"});
              g_app->m_shellOpenDirFn(dir);
            }
            UI::AddTooltipToLastItem("Show plugin folder in file explorerer.");

            ImGui::TableSetColumnIndex(5);
            ImGui::AlignTextToFramePadding();
            if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_codeIcon), btnSize))
            {
              PluginSettingsWindowPtr settingsWnd = MakeNewPtr<PluginSettingsWindow>();
              settingsWnd->SetPluginSettings(&plugin);
              settingsWnd->AddToUI();
            }
            UI::AddTooltipToLastItem("Show plugin settings.");

            ImGui::PopID();
          }
        }

        ImGui::EndTable();
      }
      ImGui::End();
    }

    void PluginWindow::LoadPluginSettings()
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