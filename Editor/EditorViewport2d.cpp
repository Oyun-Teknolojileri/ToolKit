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

#include "EditorViewport2d.h"

#include "App.h"
#include "PopupWindows.h"
#include "TopBar2d.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {
    Overlay2DTopBar* m_2dViewOptions = nullptr;

    EditorViewport2d::EditorViewport2d() {}

    EditorViewport2d::~EditorViewport2d()
    {
      SafeDel(m_anchorMode);
      if (m_2dViewOptions)
      {
        SafeDel(m_2dViewOptions);
      }
    }

    void EditorViewport2d::Init(Vec2 size)
    {
      EditorViewport::Init(size);
      InitViewport();
    }

    Window::Type EditorViewport2d::GetType() const { return Type::Viewport2d; }

    void EditorViewport2d::Update(float deltaTime)
    {
      if (!IsActive())
      {
        // Always update the anchor.
        m_anchorMode->Update(deltaTime);
        SDL_GetGlobalMouseState(&m_mousePosBegin.x, &m_mousePosBegin.y);
        return;
      }

      // Resize Grid
      g_app->m_2dGrid->Resize(g_max2dGridSize, AxisLabel::XY, (float) (m_gridCellSizeByPixel), 2.0f);

      PanZoom(deltaTime);
      m_anchorMode->Update(deltaTime);
    }

    void EditorViewport2d::OnResizeContentArea(float width, float height)
    {
      m_wndContentAreaSize.x = width;
      m_wndContentAreaSize.y = height;

      ResetViewportImage(GetRenderTargetSettings());
      AdjustZoom(TK_FLT_MIN);
    }

    void EditorViewport2d::DispatchSignals() const
    {
      if (!CanDispatchSignals() || m_mouseOverOverlay)
      {
        return;
      }

      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      {
        m_anchorMode->Signal(BaseMod::m_leftMouseBtnDownSgnl);
      }

      if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
      {
        m_anchorMode->Signal(BaseMod::m_leftMouseBtnUpSgnl);
      }

      if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
      {
        m_anchorMode->Signal(BaseMod::m_leftMouseBtnDragSgnl);
      }

      StateAnchorBase* anchState = static_cast<StateAnchorBase*>(m_anchorMode->m_stateMachine->m_currentState);

      if (anchState)
      {
        if (anchState->m_signalConsumed)
        {
          return;
        }
      }

      EditorViewport::DispatchSignals();
    }

    void EditorViewport2d::HandleDrop()
    {
      // AssetBrowser drop handling.
      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BrowserDragZone"))
        {
          const FileDragData& dragData = FolderView::GetFileDragData();
          const DirectoryEntry* entry  = dragData.Entries[0]; // get first entry

          if (entry->m_ext == LAYER)
          {
            MultiChoiceWindow::ButtonInfo openButton;
            openButton.m_name     = "Open";
            openButton.m_callback = [entry]() -> void
            {
              String fullPath = entry->GetFullPath();
              g_app->OpenScene(fullPath);
            };
            MultiChoiceWindow::ButtonInfo linkButton;
            linkButton.m_name     = "Link";
            linkButton.m_callback = [entry]() -> void
            {
              String fullPath = entry->GetFullPath();
              GetSceneManager()->GetCurrentScene()->LinkPrefab(fullPath);
            };
            MultiChoiceWindow::ButtonInfo mergeButton;
            mergeButton.m_name     = "Merge";
            mergeButton.m_callback = [entry]() -> void
            {
              String fullPath = entry->GetFullPath();
              g_app->MergeScene(fullPath);
            };
            MultiChoiceWindow* importOptionWnd = new MultiChoiceWindow("Open Scene",
                                                                       {openButton, linkButton, mergeButton},
                                                                       "Open, link or merge the scene?",
                                                                       true);

            UI::m_volatileWindows.push_back(importOptionWnd);
          }
        }
        ImGui::EndDragDropTarget();
      }
    }

    void EditorViewport2d::DrawOverlays()
    {
      if (g_app->m_showOverlayUI)
      {
        if (IsActive() || g_app->m_showOverlayUIAlways)
        {
          // Draw all overlays except 3DViewportOptions!
          for (size_t i = 0; i < m_overlays.size(); i++)
          {
            if (i == 1)
            {
              m_2dViewOptions->m_owner = this;
              m_2dViewOptions->Show();
              continue;
            }

            OverlayUI* overlay = m_overlays[i];
            if (overlay)
            {
              overlay->m_owner = this;
              overlay->Show();
            }
          }
        }
      }
    }

    void EditorViewport2d::AdjustZoom(float delta)
    {
      Camera* cam = GetCamera();
      float zoom  = cam->m_orthographicScale;
      if (delta == 0.0f && glm::equal(100.0f / m_zoomPercentage, zoom))
      {
        return;
      }

      // 0.0f and TK_FLT_MIN can be used to update lens,
      // so don't change percentage because of 0.0f or TK_FLT_MIN
      if (delta != 0.0f && delta != TK_FLT_MIN)
      {
        int8_t change = (delta < 0) ? (-1) : (1);
        if (change > 0)
        {
          if (m_zoomPercentage == 800)
          {
            g_app->m_statusMsg = "Max zoom";
            return;
          }
          m_zoomPercentage += 10;
        }
        else
        {
          if (m_zoomPercentage < 20)
          {
            g_app->m_statusMsg = "Min zoom";
            return;
          }
          m_zoomPercentage -= 10;
        }
      }

      cam->m_orthographicScale = 100.0f / m_zoomPercentage;
    }

    void EditorViewport2d::PanZoom(float deltaTime)
    {
      Camera* cam = GetCamera();

      // Adjust zoom always.
      if (m_mouseOverContentArea)
      {
        float zoom = ImGui::GetIO().MouseWheel;
        AdjustZoom(zoom);
      }

      if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
      {
        // Orbit around it.
        ImGuiIO& io   = ImGui::GetIO();
        float x       = -io.MouseDelta.x * cam->m_orthographicScale;
        float y       = io.MouseDelta.y * cam->m_orthographicScale;

        Vec3 displace = X_AXIS * x + Y_AXIS * y;
        cam->m_node->Translate(displace, TransformationSpace::TS_WORLD);
      }
    }

    void EditorViewport2d::InitViewport()
    {
      if (m_anchorMode)
      {
        m_anchorMode->UnInit();
        SafeDel(m_anchorMode);
      }

      m_anchorMode = new AnchorMod(ModId::Anchor);
      m_anchorMode->Init();

      ResetViewportImage(GetRenderTargetSettings());
      Camera* cam              = GetCamera();
      cam->m_orthographicScale = 1.0f;
      cam->m_node->SetTranslation(Z_AXIS * 10.0f);

      AdjustZoom(TK_FLT_MIN);

      if (!m_2dViewOptions)
      {
        m_2dViewOptions = new Overlay2DTopBar(this);
      }
      m_snapDeltas = Vec3(10.0f, 45.0f, 0.25f);
    }

  } // namespace Editor
} // namespace ToolKit
