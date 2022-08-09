#include "EditorViewport2d.h"

#include <algorithm>

#include "Camera.h"
#include "Renderer.h"
#include "App.h"
#include "GlobalDef.h"
#include "SDL.h"
#include "OverlayUI.h"
#include "Node.h"
#include "Primative.h"
#include "Grid.h"
#include "FolderWindow.h"
#include "ConsoleWindow.h"
#include "Gizmo.h"
#include "Mod.h"
#include "Util.h"
#include "DebugNew.h"
#include "Light.h"
#include "FileManager.h"

namespace ToolKit
{
  namespace Editor
  {
    Overlay2DViewportOptions* m_2dViewOptions = nullptr;

    EditorViewport2d::EditorViewport2d(XmlNode* node)
      : EditorViewport(node)
    {
      UpdateCanvasSize();
      Init2dCam();
      if (!m_2dViewOptions)
      {
        m_2dViewOptions = new Overlay2DViewportOptions(this);
      }
      m_snapDeltas = Vec3(10.0f, 45.0f, 0.25f);
    }

    EditorViewport2d::EditorViewport2d(float width, float height)
      : EditorViewport(width, height)
    {
      UpdateCanvasSize();
      Init2dCam();
    }

    EditorViewport2d::~EditorViewport2d()
    {
      if (m_2dViewOptions)
      {
        SafeDel(m_2dViewOptions);
      }
    }

    void EditorViewport2d::Show()
    {
      m_mouseOverOverlay = false;

      Vec2 size =
      {
        g_app->m_playWidth * 1.1f,
        g_app->m_playHeight * 1.1f
      };

      ImGui::SetNextWindowSize(size, ImGuiCond_Once);
      if
      (
        ImGui::Begin
        (
          m_name.c_str(),
          &m_visible,
          ImGuiWindowFlags_NoScrollWithMouse
          | ImGuiWindowFlags_HorizontalScrollbar
        )
      )
      {
        UpdateContentArea();
        UpdateWindow();
        HandleStates();
        DrawCommands();
        HandleDrop();
        DrawOverlays();
        if (m_mouseOverContentArea && g_app->m_snapsEnabled)
        {
          g_app->m_moveDelta = m_snapDeltas.x;
          g_app->m_rotateDelta = m_snapDeltas.y;
          g_app->m_scaleDelta = m_snapDeltas.z;
        }
      }
      ImGui::End();
    }

    Window::Type EditorViewport2d::GetType() const
    {
      return Type::Viewport2d;
    }

    void EditorViewport2d::Update(float deltaTime)
    {
      if (!IsActive())
      {
        SDL_GetGlobalMouseState(&m_mousePosBegin.x, &m_mousePosBegin.y);
        return;
      }

      // Resize Grid
      g_app->m_2dGrid->Resize
      (
        m_gridWholeSize,
        AxisLabel::XY,
        m_gridCellSizeByPixel
      );

      PanZoom(deltaTime);
    }

    void EditorViewport2d::OnResize(float width, float height)
    {
      m_width = width;
      m_height = height;

      m_viewportImage->UnInit();
      m_viewportImage->m_width = (uint)m_canvasSize.x;
      m_viewportImage->m_height = (uint)m_canvasSize.y;
      m_viewportImage->Init();

      AdjustZoom(FLT_MIN);
    }

    Vec2 EditorViewport2d::GetLastMousePosViewportSpace()
    {
      // Convert relative to canvas.
      Vec2 screenPoint = m_lastMousePosRelContentArea - IVec2(m_canvasPos);
      screenPoint.y = m_canvasSize.y - screenPoint.y;  // Convert to viewport.

      return screenPoint;
    }

    Vec2 EditorViewport2d::GetLastMousePosScreenSpace()
    {
      Vec2 viewportPoint = GetLastMousePosViewportSpace();
      viewportPoint.y = m_canvasSize.y - viewportPoint.y;
      Vec2 screenPoint = m_wndPos + m_canvasPos + viewportPoint;
      return screenPoint;
    }

    Vec3 EditorViewport2d::TransformViewportToWorldSpace(const Vec2& pnt)
    {
      Vec3 screenPoint = Vec3(pnt, 0.0f);

      Camera* cam = GetCamera();
      Mat4 view = cam->GetViewMatrix();
      Mat4 project = cam->GetProjectionMatrix();

      return glm::unProject
      (
        screenPoint,
        view, project,
        Vec4(0.0f, 0.0f, m_canvasSize.x, m_canvasSize.y)
      );
    }

    Vec2 EditorViewport2d::TransformScreenToViewportSpace(const Vec2& pnt)
    {
      Vec2 vp = pnt - m_contentAreaMin - m_canvasPos;  // In canvas space.
      vp.y = m_canvasSize.y - vp.y;  // In viewport space.

      return vp;
    }

    void EditorViewport2d::GetContentAreaScreenCoordinates
    (
      Vec2* min,
      Vec2* max
    ) const
    {
      *min = m_canvasPos + m_wndPos;
      *max = *min + m_canvasSize;
    }

    void EditorViewport2d::UpdateContentArea()
    {
      // Content area size
      m_contentAreaMin = ImGui::GetWindowContentRegionMin();
      m_contentAreaMax = ImGui::GetWindowContentRegionMax();

      m_contentAreaMin.x += ImGui::GetWindowPos().x;
      m_contentAreaMin.y += ImGui::GetWindowPos().y;
      m_contentAreaMax.x += ImGui::GetWindowPos().x;
      m_contentAreaMax.y += ImGui::GetWindowPos().y;

      m_wndPos.x = m_contentAreaMin.x;
      m_wndPos.y = m_contentAreaMin.y;

      m_wndContentAreaSize = Vec2
      (
        glm::abs(m_contentAreaMax.x - m_contentAreaMin.x),
        glm::abs(m_contentAreaMax.y - m_contentAreaMin.y)
      );

      m_canvasSize = m_wndContentAreaSize * 0.8f;

      ImGuiIO& io = ImGui::GetIO();
      ImVec2 absMousePos = io.MousePos;
      m_mouseOverContentArea = false;
      if
      (
        m_contentAreaMin.x < absMousePos.x &&
        m_contentAreaMax.x > absMousePos.x
      )
      {
        if
        (
          m_contentAreaMin.y < absMousePos.y &&
          m_contentAreaMax.y > absMousePos.y
        )
        {
          m_mouseOverContentArea = true;
        }
      }

      m_lastMousePosRelContentArea.x =
      static_cast<int>(absMousePos.x - m_contentAreaMin.x);
      m_lastMousePosRelContentArea.y =
      static_cast<int>(absMousePos.y - m_contentAreaMin.y);
    }

    void EditorViewport2d::UpdateWindow()
    {
      if (!ImGui::IsWindowCollapsed())
      {
        m_scroll = Vec2(ImGui::GetScrollX(), ImGui::GetScrollY());
        if (m_wndContentAreaSize.x > 0 && m_wndContentAreaSize.y > 0)
        {
          // Resize window.
          if
          (
            glm::notEqual(m_wndContentAreaSize.x, m_width) ||
            glm::notEqual(m_wndContentAreaSize.y, m_height)
          )
          {
            OnResize(m_wndContentAreaSize.x, m_wndContentAreaSize.y);
          }

          Vec2 wndSize = ImGui::GetWindowSize();
          ImGui::SetCursorPos((wndSize - m_canvasSize) * 0.5f);
          ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
          ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

          ImDrawList* dw = ImGui::GetWindowDrawList();
          dw->ChannelsSplit(2);
          dw->ChannelsSetCurrent(1);

          ImGui::BeginChildFrame
          (
            ImGui::GetID("canvas"),
            m_canvasSize,
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_AlwaysUseWindowPadding
          );
          m_canvasPos = ImGui::GetWindowPos();
          ImGui::Image
          (
            Convert2ImGuiTexture(m_viewportImage),
            m_canvasSize,
            ImVec2(0.0f, 0.0f),
            ImVec2(1.0f, -1.0f)
          );

          dw->ChannelsSetCurrent(0);

          m_canvasPos -= m_contentAreaMin;  // Convert relative to content area.
          ImGui::EndChildFrame();
          ImGui::PopStyleVar(3);

          dw->ChannelsMerge();

          // Draw borders.
          if (IsActive())
          {
            ImGui::GetWindowDrawList()->AddRect
            (
              m_contentAreaMin + m_scroll,
              m_contentAreaMax + m_scroll,
              IM_COL32(255, 255, 0, 255)
            );
          }
          else
          {
            ImGui::GetWindowDrawList()->AddRect
            (
              m_contentAreaMin + m_scroll,
              m_contentAreaMax + m_scroll,
              IM_COL32(128, 128, 128, 255)
            );
          }
        }
      }

      m_mouseHover = ImGui::IsWindowHovered();
    }

    void EditorViewport2d::DrawCommands()
    {
      // Process draw commands.
      ImDrawList* drawList = ImGui::GetForegroundDrawList();
      for (auto command : m_drawCommands)
      {
        command(drawList);
      }
      m_drawCommands.clear();
    }

    void EditorViewport2d::HandleDrop()
    {
      // AssetBrowser drop handling.
      if (ImGui::BeginDragDropTarget())
      {
        if
        (
          const ImGuiPayload* payload =
          ImGui::AcceptDragDropPayload("BrowserDragZone")
        )
        {
          IM_ASSERT(payload->DataSize == sizeof(DirectoryEntry));
          DirectoryEntry entry = *(const DirectoryEntry*)payload->Data;

          if (entry.m_ext == MESH)
          {
            String path = ConcatPaths
            (
              { entry.m_rootPath, entry.m_fileName + entry.m_ext }
            );

            ImGuiIO& io = ImGui::GetIO();
            Drawable* dwMesh = new Drawable();
            if (io.KeyShift)
            {
              MeshPtr mesh = GetMeshManager()->Create<Mesh>(path);
              dwMesh->SetMesh(mesh->Copy<Mesh>());
            }
            else
            {
              dwMesh->SetMesh(GetMeshManager()->Create<Mesh>(path));
            }

            dwMesh->GetMesh()->Init(false);
            Ray ray = RayFromMousePosition();
            Vec3 pos = PointOnRay(ray, 5.0f);
            g_app->m_grid->HitTest(ray, pos);
            dwMesh->m_node->SetTranslation(pos);
            EditorScenePtr currScene = g_app->GetCurrentScene();
            currScene->AddEntity(dwMesh);
            currScene->AddToSelection(dwMesh->GetIdVal(), false);
            SetActive();
          }
          else if (entry.m_ext == SCENE)
          {
            YesNoWindow* importOptionWnd = new YesNoWindow
            (
              "Open Scene",
              "Open",
              "Merge",
              "Open or merge the scene ?",
              true
            );
            importOptionWnd->m_yesCallback = [entry]() ->void
            {
              String fullPath = entry.GetFullPath();
              g_app->OpenScene(fullPath);
            };

            importOptionWnd->m_noCallback = [entry]() -> void
            {
              String fullPath = entry.GetFullPath();
              g_app->MergeScene(fullPath);
            };

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
          for (uint32_t i = 0; i < m_overlays.size(); i++)
          {
            if (i == 1)
            {
              m_2dViewOptions->m_scroll = m_scroll;
              m_2dViewOptions->m_owner = this;
              m_2dViewOptions->Show();
              m_2dViewOptions->m_scroll = Vec2();
              continue;
            }

            OverlayUI* overlay = m_overlays[i];
            if (overlay)
            {
              overlay->m_scroll = m_scroll;
              overlay->m_owner = this;
              overlay->Show();
              overlay->m_scroll = Vec2();
            }
          }
        }
      }
    }

    void EditorViewport2d::DrawCanvasToolBar()
    {
    }

    void EditorViewport2d::AdjustZoom(float delta)
    {
      if (delta == 0.0f && 100.0f / m_zoomPercentage == m_zoom)
      {
        return;
      }
      // 0.0f and FLT_MIN can be used to update lens,
      // so don't change percentage because of 0.0f or FLT_MIN
      if (delta != 0.0f && delta != FLT_MIN)
      {
        int8_t change = (delta < 0) ? (-1) : (1);
        if (change > 0)
        {
          if (m_zoomPercentage == 800)
          {
            g_app->m_statusMsg = "Max zoom";
            return;
          }
          if (m_zoomPercentage >= 100)
          {
            m_zoomPercentage += 100;
          }
          else if (m_zoomPercentage < 100)
          {
            m_zoomPercentage *= 2;
          }
        }
        else
        {
          if (m_zoomPercentage > 100)
          {
            m_zoomPercentage -= 100;
          }
          else if (m_zoomPercentage <= 100 && m_zoomPercentage > 13)
          {
            m_zoomPercentage /= 2;
          }
          else
          {
            g_app->m_statusMsg = "Min zoom";
          }
        }
      }
      m_zoom = 100.0f / m_zoomPercentage;
      GetCamera()->SetLens
      (
        m_canvasSize.x * m_zoom * -0.5f,
        m_canvasSize.x * m_zoom * 0.5f,
        m_canvasSize.y * m_zoom * -0.5f,
        m_canvasSize.y * m_zoom * 0.5f,
        0.01f,
        1000.0f
      );
    }

    void EditorViewport2d::Init2dCam()
    {
      m_zoom = 1.0f;
      GetCamera()->m_node->SetTranslation(Z_AXIS * 10.0f);
      AdjustZoom(FLT_MIN);
    }

    void EditorViewport2d::UpdateCanvasSize()
    {
      m_canvasSize =
      {
        g_app->m_playWidth,
        g_app->m_playHeight
      };

      m_viewportImage->UnInit();
      m_viewportImage->m_width = (uint)m_canvasSize.x;
      m_viewportImage->m_height = (uint)m_canvasSize.y;
      m_viewportImage->Init();
    }

    void EditorViewport2d::PanZoom(float deltaTime)
    {
      Camera* cam = GetCamera();
      if (cam)
      {
        // Adjust zoom always.
        if (m_mouseOverContentArea)
        {
          float zoom = ImGui::GetIO().MouseWheel;
          AdjustZoom(zoom);
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
          // Orbit around it.
          ImGuiIO& io = ImGui::GetIO();
          float x = -io.MouseDelta.x * m_zoom;
          float y = io.MouseDelta.y * m_zoom;

          Vec3 displace = X_AXIS * x + Y_AXIS * y;
          cam->m_node->Translate(displace, TransformationSpace::TS_WORLD);
        }
      }
    }
  }  // namespace Editor
}  // namespace ToolKit
