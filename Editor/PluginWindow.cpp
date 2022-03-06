#include "stdafx.h"
#include "PluginWindow.h"
#include "GlobalDef.h"
#include "ConsoleWindow.h"
#include "App.h"
#include "DebugNew.h"

namespace ToolKit
{

  namespace Editor
  {

    PluginWindow::PluginWindow()
    {
      m_name = "Plugin";
    }

    PluginWindow::PluginWindow(XmlNode* node)
    {
      DeSerialize(nullptr, node);
    }

    PluginWindow::~PluginWindow()
    {
    }

    void PluginWindow::Show()
    {
      ImGui::SetNextWindowSize(ImVec2(300, 30), ImGuiCond_Once);
      if 
      (
        ImGui::Begin
        (
          "Plugin",
          &m_visible,
          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
        )
      )
      {
        HandleStates();

        Vec2 min = ImGui::GetWindowContentRegionMin();
        Vec2 max = ImGui::GetWindowContentRegionMax();
        Vec2 size = max - min;

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Game");
        ImVec2 tsize = ImGui::CalcTextSize("Game");

        ImGui::SameLine();

        // Draw play - pause - stop buttons.
        float btnWidth = 24.0f;
        float offset = (size.x - btnWidth * 2.0f - tsize.x) * 0.5f;
        
        ImGui::SameLine(offset);

        if (g_app->m_gameMod == App::GameMod::Playing)
        {
          // Blue tint.
          ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(4 / 7.0f, 0.6f, 0.6f));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(4 / 7.0f, 0.7f, 0.7f));
          ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(4 / 7.0f, 0.8f, 0.8f));

          // Pause.
          if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_pauseIcon), ImVec2(btnWidth, btnWidth)))
          {
            g_app->SetGameMod(App::GameMod::Paused);
          }

          ImGui::PopStyleColor(3);
        }
        else
        {
          // Green tint.
          ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2 / 7.0f, 0.7f, 0.7f));
          ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.8f));

          // Play.
          if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_playIcon), ImVec2(btnWidth, btnWidth)))
          {
            g_app->SetGameMod(App::GameMod::Playing);
          }

          ImGui::PopStyleColor(3);
        }

        ImGui::SameLine();

        // Red tint.
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.8f));

        // Stop.
        if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_stopIcon), ImVec2(btnWidth, btnWidth)))
        {
          if (g_app->m_gameMod != App::GameMod::Stop)
          {
            g_app->SetGameMod(App::GameMod::Stop);
          }
        }

        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        
        if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_vsCodeIcon), ImVec2(btnWidth, btnWidth)))
        {
          String codePath = g_app->m_workspace.GetCodePath();
          if (CheckFile(codePath))
          {
            String cmd = "code \"" + codePath + "\"";
            int result = std::system(cmd.c_str());
            if (result != 0)
            {
              g_app->GetConsole()->AddLog("Visual Studio Code can't be started. Make sure it is installed.", ConsoleWindow::LogType::Error);
            }
          }
          else
          {
            g_app->GetConsole()->AddLog("There is not a vaild code folder.", ConsoleWindow::LogType::Error);
          }
        }

        // Other editor and game entity plugins.
        ImGui::Separator();
        
        ImGui::Checkbox("Run in window", &g_app->m_runWindowed);
        if (ImGui::BeginTable("EmuSet", 4, ImGuiTableFlags_SizingFixedFit))
        {
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("Width: ");
          ImGui::TableSetColumnIndex(1);
          ImGui::SetNextItemWidth(100.0f);
          ImGui::InputFloat("##w", &g_app->m_playWidth, 0, 0, "%.0f");
          ImGui::TableSetColumnIndex(2);
          ImGui::Text("Height: ");
          ImGui::TableSetColumnIndex(3);
          ImGui::SetNextItemWidth(100.0f);
          ImGui::InputFloat("##h", &g_app->m_playHeight, 0, 0, "%.0f");

          ImGui::EndTable();
        }
      }

      ImGui::End();
    }

    Window::Type PluginWindow::GetType() const
    {
      return Type::PluginWindow;
    }

    void PluginWindow::Serialize(XmlDocument* doc, XmlNode* parent) const
    {
      Window::Serialize(doc, parent);
    }

    void PluginWindow::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      Window::DeSerialize(doc, parent);
    }

  }

}
