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

#include "TopBar2d.h"

#include "App.h"
#include "EditorScene.h"
#include "EditorViewport2d.h"

#include <Canvas.h>
#include <Surface.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {

    Overlay2DTopBar::Overlay2DTopBar(EditorViewport* owner) : OverlayTopBar(owner) {}

    void Overlay2DTopBar::Show()
    {
      assert(m_owner);
      if (m_owner == nullptr)
      {
        return;
      }

      auto ShowAddMenuFn = []() -> void
      {
        EditorScenePtr currScene = g_app->GetCurrentScene();

        if (ImGui::MenuItem("Surface"))
        {
          SurfacePtr surface = MakeNewPtr<Surface>();
          surface->Update(Vec2(100.0f, 30.0f), Vec2(0.0f));
          surface->GetMeshComponent()->Init(false);
          currScene->AddEntity(surface);
        }

        if (ImGui::MenuItem("Button"))
        {
          ButtonPtr btn = MakeNewPtr<Button>();
          btn->Update(Vec2(100.0f, 30.0f));
          btn->GetMeshComponent()->Init(false);
          currScene->AddEntity(btn);
        }

        if (ImGui::MenuItem("Canvas"))
        {
          CanvasPtr canvasPanel = MakeNewPtr<Canvas>();
          canvasPanel->Update(Vec2(800.0f, 600.0f));
          canvasPanel->SetPivotOffsetVal(Vec2(0.5f));
          canvasPanel->GetMeshComponent()->Init(false);
          currScene->AddEntity(canvasPanel);
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Node"))
        {
          EntityPtr node = MakeNewPtr<EntityNode>();
          currScene->AddEntity(node);
        }
      };

      ImVec2 overlaySize(300, 30);

      // Center the toolbar.
      float width  = ImGui::GetWindowContentRegionWidth();
      float offset = (width - overlaySize.x) * 0.5f;
      ImGui::SameLine(offset);

      ImGui::SetNextWindowBgAlpha(0.85f);
      if (ImGui::BeginChildFrame(ImGui::GetID("ViewportOptions"),
                                 overlaySize,
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoScrollWithMouse))
      {
        SetOwnerState();

        ImGui::BeginTable("##SettingsBar", 8, ImGuiTableFlags_SizingStretchProp);
        ImGui::TableNextRow();

        uint nextItemIndex = 0;

        ShowAddMenu(ShowAddMenuFn, nextItemIndex);

        ShowTransformOrientation(nextItemIndex);

        SnapOptions(nextItemIndex);

        Show2DViewZoomOptions(nextItemIndex);

        ShowGridOptions(nextItemIndex);

        ImGui::EndTable();
      }
      ImGui::EndChildFrame();
    }

    void Overlay2DTopBar::Show2DViewZoomOptions(uint32_t& nextItemIndex)
    {
      EditorViewport2d* editorViewport = reinterpret_cast<EditorViewport2d*>(m_owner);
      ImGui::TableSetColumnIndex(nextItemIndex++);
      if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_viewZoomIcon), ImVec2(16, 16)))
      {
        editorViewport->m_zoomPercentage = 100;
      }
      UI::HelpMarker(TKLoc + m_owner->m_name, "Reset Zoom");
      ImGui::TableSetColumnIndex(nextItemIndex++);
      ImGui::Text("%u%%", uint32_t(editorViewport->m_zoomPercentage));
    }

    void Overlay2DTopBar::ShowGridOptions(uint32_t& nextItemIndex)
    {
      auto ShowGridOptionsFn = [this]() -> void
      {
        ImGui::PushItemWidth(75);
        EditorViewport2d* editorViewport   = reinterpret_cast<EditorViewport2d*>(m_owner);

        static const uint16_t cellSizeStep = 5;
        ImGui::InputScalar("Cell Size", ImGuiDataType_U16, &editorViewport->m_gridCellSizeByPixel, &cellSizeStep);

        editorViewport->m_gridCellSizeByPixel =
            glm::max((editorViewport->m_gridCellSizeByPixel / cellSizeStep) * cellSizeStep, 1u);
      };

      ImGui::TableSetColumnIndex(nextItemIndex++);
      if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_gridIcon), ImVec2(18, 18)))
      {
        ImGui::OpenPopup("##GridMenu");
      }
      UI::HelpMarker(TKLoc + m_owner->m_name, "Grid Options");
      if (ImGui::BeginPopup("##GridMenu"))
      {
        ShowGridOptionsFn();
        ImGui::EndPopup();
      }
    }

  } // namespace Editor
} // namespace ToolKit