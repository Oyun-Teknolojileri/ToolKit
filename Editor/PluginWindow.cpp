#include "stdafx.h"
#include "PluginWindow.h"
#include "GlobalDef.h"

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
        float btnWidth = 30.0f;
        float offset = (size.x - btnWidth * 2.0f - tsize.x) * 0.5f;
        
        ImGui::SameLine(offset);

        if (g_app->m_gameMod == App::GameMod::Playing)
        {
          // Blue tint.
          ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(4 / 7.0f, 0.6f, 0.6f));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(4 / 7.0f, 0.7f, 0.7f));
          ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(4 / 7.0f, 0.8f, 0.8f));

          // Pause.
          if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_pauseIcon), ImVec2(30, 30)))
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
          if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_playIcon), ImVec2(30, 30)))
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
        if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_stopIcon), ImVec2(30, 30)))
        {
          if (g_app->m_gameMod != App::GameMod::Stop)
          {
            g_app->SetGameMod(App::GameMod::Stop);
          }
        }

        ImGui::PopStyleColor(3);

        // Other editor and game entity plugins.
        ImGui::Separator();
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
