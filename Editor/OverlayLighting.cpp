#include "OverlayLighting.h"

#include "App.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    OverlayLighting::OverlayLighting(EditorViewport* owner) : OverlayUI(owner)
    {
    }

    void OverlayLighting::Show()
    {
      Vec2 overlaySize(28.0f, 30.0f);
      if (!m_editorLitModeOn)
      {
        overlaySize.x += 170.0f;
      }

      const float padding = 5.0f;

      // Upper right.
      Vec2 wndPos =
          m_owner->m_contentAreaLocation + m_owner->m_wndContentAreaSize;
      wndPos.y =
          m_owner->m_contentAreaLocation.y + overlaySize.y + padding * 2.0f;

      wndPos -= overlaySize;
      wndPos -= padding;

      ImGui::SetNextWindowPos(wndPos);
      ImGui::SetNextWindowBgAlpha(0.65f);

      if (ImGui::BeginChildFrame(ImGui::GetID("LightingOptions"),
                                 overlaySize,
                                 ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoTitleBar |
                                     ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoScrollWithMouse))
      {
        SetOwnerState();
        ImGui::BeginTable("##SettingsBar",
                          m_editorLitModeOn ? 1 : 2,
                          ImGuiTableFlags_SizingStretchProp);

        ImGui::TableNextRow();
        unsigned int nextItemIndex = 0;

        if (!m_editorLitModeOn)
        {
          ImGui::TableSetColumnIndex(nextItemIndex++);
          ImGui::PushItemWidth(160);
          uint lightModeIndx      = (int) g_app->m_sceneLightingMode;
          const char* itemNames[] = {"Editor Lit",
                                     "Unlit",
                                     "Full Lit",
                                     "Light Complexity",
                                     "Lighting Only",
                                     "Game"};

          uint itemCount          = sizeof(itemNames) / sizeof(itemNames[0]);
          if (ImGui::BeginCombo("", itemNames[lightModeIndx]))
          {
            for (uint itemIndx = 1; itemIndx < itemCount; itemIndx++)
            {
              bool isSelected      = false;
              const char* itemName = itemNames[itemIndx];
              ImGui::Selectable(itemName, &isSelected);
              if (isSelected)
              {
                // 0 is EditorLit
                lightModeIndx = itemIndx;
              }
            }

            ImGui::EndCombo();
          }

          ImGui::PopItemWidth();
          g_app->m_sceneLightingMode = (EditorLitMode) lightModeIndx;
        }
        else
        {
          if (g_app->m_gameMod == GameMod::Stop)
          {
            g_app->m_sceneLightingMode = EditorLitMode::EditorLit;
          }
        }

        ImGui::TableSetColumnIndex(nextItemIndex++);
        m_editorLitModeOn =
            UI::ToggleButton(UI::m_studioLightsToggleIcon->m_textureId,
                             ImVec2(12.0f, 14.0f),
                             m_editorLitModeOn);

        UI::HelpMarker(TKLoc + m_owner->m_name, "Scene Lighting Mode");
        ImGui::EndTable();
      }
      ImGui::EndChildFrame();
    }

  } // namespace Editor
} // namespace ToolKit
