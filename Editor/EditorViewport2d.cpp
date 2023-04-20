#include "EditorViewport2d.h"

#include "App.h"
#include "Camera.h"
#include "ConsoleWindow.h"
#include "FileManager.h"
#include "FolderWindow.h"
#include "Gizmo.h"
#include "Global.h"
#include "Grid.h"
#include "Light.h"
#include "Mod.h"
#include "Node.h"
#include "OverlayUI.h"
#include "PopupWindows.h"
#include "Primative.h"
#include "Renderer.h"
#include "SDL.h"
#include "TopBar2d.h"
#include "Util.h"

#include <algorithm>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    Overlay2DTopBar* m_2dViewOptions = nullptr;

    EditorViewport2d::EditorViewport2d(XmlNode* node) : EditorViewport(node)
    {
      InitViewport();
    }

    EditorViewport2d::EditorViewport2d(float width, float height)
        : EditorViewport(width, height)
    {
      InitViewport();
    }

    EditorViewport2d::EditorViewport2d(const Vec2& size)
        : EditorViewport2d(size.x, size.y)
    {
    }

    EditorViewport2d::~EditorViewport2d()
    {
      SafeDel(m_anchorMode);
      if (m_2dViewOptions)
      {
        SafeDel(m_2dViewOptions);
      }
    }

    Window::Type EditorViewport2d::GetType() const { return Type::Viewport2d; }

    void EditorViewport2d::Update(float deltaTime)
    {
      if (!IsActive())
      {
        SDL_GetGlobalMouseState(&m_mousePosBegin.x, &m_mousePosBegin.y);
        return;
      }

      // Resize Grid
      g_app->m_2dGrid->Resize(g_max2dGridSize,
                              AxisLabel::XY,
                              (float) (m_gridCellSizeByPixel),
                              2.0f);

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

      StateAnchorBase* anchState = static_cast<StateAnchorBase*>(
          m_anchorMode->m_stateMachine->m_currentState);

      if (anchState)
      {
        if (anchState->m_signalConsumed)
        {
          return;
        }
      }

      EditorViewport::DispatchSignals();
    }

    Vec2 EditorViewport2d::GetLastMousePosViewportSpace()
    {
      // Convert relative to canvas.
      Vec2 screenPoint = m_lastMousePosRelContentArea - IVec2(m_canvasPos);
      screenPoint.y    = m_canvasSize.y - screenPoint.y; // Convert to viewport.

      return screenPoint;
    }

    Vec2 EditorViewport2d::GetLastMousePosScreenSpace()
    {
      Vec2 viewportPoint = GetLastMousePosViewportSpace();
      viewportPoint.y    = m_canvasSize.y - viewportPoint.y;
      Vec2 screenPoint   = m_contentAreaLocation + m_canvasPos + viewportPoint;
      return screenPoint;
    }

    Vec3 EditorViewport2d::TransformViewportToWorldSpace(const Vec2& pnt)
    {
      Vec3 screenPoint = Vec3(pnt, 0.0f);

      Camera* cam      = GetCamera();
      Mat4 view        = cam->GetViewMatrix();
      Mat4 project     = cam->GetProjectionMatrix();

      return glm::unProject(screenPoint,
                            view,
                            project,
                            Vec4(0.0f, 0.0f, m_canvasSize.x, m_canvasSize.y));
    }

    Vec2 EditorViewport2d::TransformScreenToViewportSpace(const Vec2& pnt)
    {
      Vec2 vp = pnt - m_contentAreaMin - m_canvasPos; // In canvas space.
      vp.y    = m_canvasSize.y - vp.y;                // In viewport space.

      return vp;
    }

    void EditorViewport2d::GetContentAreaScreenCoordinates(Vec2* min,
                                                           Vec2* max) const
    {
      *min = m_canvasPos + m_contentAreaLocation;
      *max = *min + m_canvasSize;
    }

    void EditorViewport2d::UpdateContentArea()
    {
      EditorViewport::UpdateContentArea();
      m_canvasSize = m_wndContentAreaSize;
    }

    void EditorViewport2d::UpdateWindow()
    {
      EditorViewport::UpdateWindow();
      return;

      if (!ImGui::IsWindowCollapsed())
      {
        // Resize window.
        Vec2 wndSize = ImGui::GetWindowSize();
        if (!VecAllEqual(wndSize, Vec2(m_size)))
        {
          ResizeWindow((uint) wndSize.x, (uint) wndSize.y);
        }

        if (m_wndContentAreaSize.x > 0 && m_wndContentAreaSize.y > 0)
        {
          ImGui::SetCursorPos((wndSize - m_canvasSize) * 0.5f);
          ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
          ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
          ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                ImGui::GetStyleColorVec4(ImGuiCol_ChildBg));

          ImDrawList* dw = ImGui::GetWindowDrawList();
          dw->ChannelsSplit(2);
          dw->ChannelsSetCurrent(1);

          ImGui::BeginChildFrame(ImGui::GetID("canvas"),
                                 m_canvasSize,
                                 ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoScrollWithMouse |
                                     ImGuiWindowFlags_AlwaysUseWindowPadding);
          m_canvasPos = ImGui::GetWindowPos();

          uint texId  = 0;
          if (m_framebuffer->GetAttachment(
                  Framebuffer::Attachment::ColorAttachment0) != nullptr)
          {
            texId =
                m_framebuffer
                    ->GetAttachment(Framebuffer::Attachment::ColorAttachment0)
                    ->m_textureId;
          }

          ImGui::Image(ConvertUIntImGuiTexture(texId),
                       m_canvasSize,
                       ImVec2(0.0f, 0.0f),
                       ImVec2(1.0f, -1.0f));

          dw->ChannelsSetCurrent(0);

          m_canvasPos -= m_contentAreaMin; // Convert relative to content area.
          ImGui::EndChildFrame();

          dw->ChannelsMerge();
          ImGui::PopStyleColor(1);
          ImGui::PopStyleVar(3);

          // Draw borders.
          if (IsActive())
          {
            ImGui::GetWindowDrawList()->AddRect(m_contentAreaMin,
                                                m_contentAreaMax,
                                                IM_COL32(255, 255, 0, 255));
          }
          else
          {
            ImGui::GetWindowDrawList()->AddRect(m_contentAreaMin,
                                                m_contentAreaMax,
                                                IM_COL32(128, 128, 128, 255));
          }

          GetUIManager()->UpdateLayers(g_app->GetDeltaTime(), this);
        }
      }

      m_mouseHover = ImGui::IsWindowHovered();
    }

    void EditorViewport2d::HandleDrop()
    {
      // AssetBrowser drop handling.
      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("BrowserDragZone"))
        {
          IM_ASSERT(payload->DataSize == sizeof(DirectoryEntry));
          DirectoryEntry entry = *(const DirectoryEntry*) payload->Data;

          if (entry.m_ext == LAYER)
          {
            MultiChoiceWindow::ButtonInfo openButton;
            openButton.m_name     = "Open";
            openButton.m_callback = [entry]() -> void
            {
              String fullPath = entry.GetFullPath();
              g_app->OpenScene(fullPath);
            };
            MultiChoiceWindow::ButtonInfo linkButton;
            linkButton.m_name     = "Link";
            linkButton.m_callback = [entry]() -> void
            {
              String fullPath = entry.GetFullPath();
              GetSceneManager()->GetCurrentScene()->LinkPrefab(fullPath);
            };
            MultiChoiceWindow::ButtonInfo mergeButton;
            mergeButton.m_name     = "Merge";
            mergeButton.m_callback = [entry]() -> void
            {
              String fullPath = entry.GetFullPath();
              g_app->MergeScene(fullPath);
            };
            MultiChoiceWindow* importOptionWnd =
                new MultiChoiceWindow("Open Scene",
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
