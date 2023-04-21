#include "LeftBar.h"

#include "App.h"
#include "Mod.h"

namespace ToolKit
{
  namespace Editor
  {

    OverlayLeftBar::OverlayLeftBar(EditorViewport* owner) : OverlayUI(owner) {}

    void OverlayLeftBar::Show()
    {
      const float padding = 5.0f;
      Vec2 wndPos         = Vec2(m_owner->m_contentAreaLocation.x + padding,
                         m_owner->m_contentAreaLocation.y + padding);

      ImGui::SetNextWindowPos(wndPos);
      ImGui::SetNextWindowBgAlpha(0.65f);

      static ImVec2 overlaySize(48, 0); // Set initial height to 0
      if (ImGui::BeginChildFrame(
              ImGui::GetID("Navigation"),
              overlaySize,
              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking |
                  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                  ImGuiWindowFlags_NoSavedSettings |
                  ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                  ImGuiWindowFlags_NoScrollbar |
                  ImGuiWindowFlags_NoScrollWithMouse))
      {
        SetOwnerState();

        // Select button.
        bool isCurrentMod =
            ModManager::GetInstance()->m_modStack.back()->m_id == ModId::Select;
        ModManager::GetInstance()->SetMod(
            UI::ToggleButton(UI::m_selectIcn->m_textureId,
                             ImVec2(32, 32),
                             isCurrentMod) &&
                !isCurrentMod,
            ModId::Select);
        UI::HelpMarker(TKLoc + m_owner->m_name,
                       "Select Box\nSelect items using box selection.");

        // Cursor button.
        isCurrentMod =
            ModManager::GetInstance()->m_modStack.back()->m_id == ModId::Cursor;
        ModManager::GetInstance()->SetMod(
            UI::ToggleButton(UI::m_cursorIcn->m_textureId,
                             ImVec2(32, 32),
                             isCurrentMod) &&
                !isCurrentMod,
            ModId::Cursor);
        UI::HelpMarker(TKLoc + m_owner->m_name,
                       "Cursor\nSet the cursor location.");
        ImGui::Separator();

        // Move button.
        isCurrentMod =
            ModManager::GetInstance()->m_modStack.back()->m_id == ModId::Move;
        ModManager::GetInstance()->SetMod(
            UI::ToggleButton(UI::m_moveIcn->m_textureId,
                             ImVec2(32, 32),
                             isCurrentMod) &&
                !isCurrentMod,
            ModId::Move);
        UI::HelpMarker(TKLoc + m_owner->m_name, "Move\nMove selected items.");

        // Rotate button.
        isCurrentMod =
            ModManager::GetInstance()->m_modStack.back()->m_id == ModId::Rotate;
        ModManager::GetInstance()->SetMod(
            UI::ToggleButton(UI::m_rotateIcn->m_textureId,
                             ImVec2(32, 32),
                             isCurrentMod) &&
                !isCurrentMod,
            ModId::Rotate);
        UI::HelpMarker(TKLoc + m_owner->m_name,
                       "Rotate\nRotate selected items.");

        // Scale button.
        isCurrentMod =
            ModManager::GetInstance()->m_modStack.back()->m_id == ModId::Scale;
        ModManager::GetInstance()->SetMod(
            UI::ToggleButton(UI::m_scaleIcn->m_textureId,
                             ImVec2(32, 32),
                             isCurrentMod) &&
                !isCurrentMod,
            ModId::Scale);
        UI::HelpMarker(TKLoc + m_owner->m_name,
                       "Scale\nScale (resize) selected items.");
        ImGui::Separator();

        const char* items[]     = {"1", "2", "4", "8", "16"};
        static int currentItem = 3; // Also the default.
        ImGui::PushItemWidth(40);
        if (ImGui::BeginCombo("##CS",
                              items[currentItem],
                              ImGuiComboFlags_None))
        {
          for (int n = 0; n < IM_ARRAYSIZE(items); n++)
          {
            bool isSelected = (currentItem == n);
            if (ImGui::Selectable(items[n], isSelected))
            {
              currentItem = n;
            }

            if (isSelected)
            {
              ImGui::SetItemDefaultFocus();
            }
          }
          ImGui::EndCombo();
        }
        Vec2 comboHeight = ImGui::GetItemRectSize();

        ImGui::PopItemWidth();

        switch (currentItem)
        {
        case 0:
          g_app->m_camSpeed = 0.5f;
          break;
        case 1:
          g_app->m_camSpeed = 1.0f;
          break;
        case 2:
          g_app->m_camSpeed = 2.0f;
          break;
        case 3:
          g_app->m_camSpeed = 4.0f;
          break;
        case 4:
          g_app->m_camSpeed = 16.0f;
          break;
        default:
          g_app->m_camSpeed = 8;
          break;
        }

        ImGuiStyle& style = ImGui::GetStyle();
        float spacing     = style.ItemInnerSpacing.x;

        ImGui::SameLine(0, spacing);
        UI::HelpMarker(TKLoc + m_owner->m_name, "Camera speed m/s\n");

        // Calculate the height of the child frame based on its content
        overlaySize.y =
            ImGui::GetCursorPosY() + style.ItemSpacing.y + comboHeight.y;
      }
      ImGui::EndChildFrame();
    }

  } // namespace Editor
} // namespace ToolKit