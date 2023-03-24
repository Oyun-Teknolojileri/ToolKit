#include "StatusBar.h"

#include "App.h"
#include "EditorViewport.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    StatusBar::StatusBar(EditorViewport* owner) : OverlayUI(owner) {}

    void StatusBar::Show()
    {
      // Status bar.
      Vec2 wndPadding = ImGui::GetStyle().WindowPadding;
      ImVec2 overlaySize;
      overlaySize.x = m_owner->m_wndContentAreaSize.x - 2.0f;
      overlaySize.y = 24;
      Vec2 pos      = m_owner->m_contentAreaLocation;

      pos.x         += 1;
      pos.y         += m_owner->m_wndContentAreaSize.y - wndPadding.y - 16.0f;
      ImGui::SetNextWindowPos(pos);
      ImGui::SetNextWindowBgAlpha(0.65f);

      if (ImGui::BeginChildFrame(ImGui::GetID("ProjectInfo"),
                                 overlaySize,
                                 ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoTitleBar |
                                     ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoScrollWithMouse))
      {
        String info = "Status: ";
        ImGui::Text(info.c_str());

        // If the status message has changed.
        static String prevMsg = g_app->m_statusMsg;
        bool nte = EndsWith(g_app->m_statusMsg, g_statusNoTerminate);
        if (g_app->m_statusMsg != "OK" && !nte)
        {
          // Hold msg for 3 sec. before switching to OK.
          static float elapsedTime = 0.0f;
          elapsedTime              += ImGui::GetIO().DeltaTime;

          // For overlapping message updates,
          // always reset timer for the last event.
          if (prevMsg != g_app->m_statusMsg)
          {
            elapsedTime = 0.0f;
            prevMsg     = g_app->m_statusMsg;
          }

          if (elapsedTime > 3)
          {
            elapsedTime        = 0.0f;
            g_app->m_statusMsg = "OK";
          }
        }

        // Inject status.
        ImGui::SameLine();

        if (nte)
        {
          String trimmed =
              Trim(g_app->m_statusMsg.c_str(), g_statusNoTerminate);
          ImGui::Text(trimmed.c_str());
        }
        else
        {
          ImGui::Text(g_app->m_statusMsg.c_str());
        }

        ImVec2 msgSize = ImGui::CalcTextSize(g_app->m_statusMsg.c_str());
        float wndWidth = ImGui::GetWindowContentRegionWidth();

        // If there is enough space for info.
        if (wndWidth * 0.3f > msgSize.x)
        {
          // Draw Projcet Info.
          Project prj = g_app->m_workspace.GetActiveProject();
          info        = "Project: " + prj.name + "Scene: " + prj.scene;
          pos         = ImGui::CalcTextSize(info.c_str());

          ImGui::SameLine((m_owner->m_wndContentAreaSize.x - pos.x) * 0.5f);
          info = "Project: " + prj.name;
          ImGui::BulletText(info.c_str());
          ImGui::SameLine();
          info = "Scene: " + prj.scene;
          ImGui::BulletText(info.c_str());

          // Draw Fps.
          String fps = "Fps: " + std::to_string(g_app->m_fps);
          ImGui::SameLine(m_owner->m_wndContentAreaSize.x - 70.0f);
          ImGui::Text(fps.c_str());
        }
      }
      ImGui::EndChildFrame();
    }

  } // namespace Editor
} // namespace ToolKit
