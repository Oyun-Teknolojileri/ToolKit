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

#include "OverlayLighting.h"

#include "App.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    OverlayLighting::OverlayLighting(EditorViewport* owner) : OverlayUI(owner) {}

    void OverlayLighting::Show()
    {
      Vec2 overlaySize(28.0f, 30.0f);
      if (!m_editorLitModeOn)
      {
        overlaySize.x += 170.0f;
      }

      const float padding = 5.0f;

      // Upper right.
      Vec2 wndPos         = m_owner->m_contentAreaLocation + m_owner->m_wndContentAreaSize;
      wndPos.y            = m_owner->m_contentAreaLocation.y + overlaySize.y + padding * 2.0f;

      wndPos              -= overlaySize;
      wndPos              -= padding;

      ImGui::SetNextWindowPos(wndPos);
      ImGui::SetNextWindowBgAlpha(0.65f);

      if (ImGui::BeginChildFrame(ImGui::GetID("LightingOptions"),
                                 overlaySize,
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoScrollWithMouse))
      {
        SetOwnerState();
        ImGui::BeginTable("##SettingsBar", m_editorLitModeOn ? 1 : 2, ImGuiTableFlags_SizingStretchProp);

        ImGui::TableNextRow();
        unsigned int nextItemIndex = 0;

        if (!m_editorLitModeOn)
        {
          ImGui::TableSetColumnIndex(nextItemIndex++);
          ImGui::PushItemWidth(160);
          uint lightModeIndx      = (int) g_app->m_sceneLightingMode;
          const char* itemNames[] = {"Editor Lit", "Unlit", "Full Lit", "Light Complexity", "Lighting Only", "Game"};

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
        m_editorLitModeOn = UI::ToggleButton(ICON_FA_LIGHTBULB, ImVec2(20.0f, 20.0f), m_editorLitModeOn);

        UI::HelpMarker(TKLoc + m_owner->m_name, "Scene Lighting Mode");
        ImGui::EndTable();
      }
      ImGui::EndChildFrame();
    }

  } // namespace Editor
} // namespace ToolKit
