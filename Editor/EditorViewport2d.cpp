#include "stdafx.h"

#include "EditorViewport2d.h"
#include "Directional.h"
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

namespace ToolKit
{
  namespace Editor
  {

    EditorViewport2d::EditorViewport2d(XmlNode* node)
      : EditorViewport(node)
    {
      GetGlobalCanvasSize();
      Init2dCam();
    }

    EditorViewport2d::EditorViewport2d(float width, float height)
      : EditorViewport(width, height)
    {
      GetGlobalCanvasSize();
      Init2dCam();
    }

    EditorViewport2d::~EditorViewport2d()
    {
    }

    void EditorViewport2d::Show()
    {
      m_mouseOverOverlay = false;

      Vec2 size =
      {
        g_app->m_playWidth,
        g_app->m_playHeight
      };

      ImGui::SetNextWindowSize(size, ImGuiCond_Once);
      if 
      (
        ImGui::Begin
        (
          m_name.c_str(), 
          &m_visible, 
          ImGuiWindowFlags_NoScrollWithMouse
        )
      )
      {
        HandleStates();

        // Content area size
        ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        ImVec2 vMax = ImGui::GetWindowContentRegionMax();

        vMin.x += ImGui::GetWindowPos().x;
        vMin.y += ImGui::GetWindowPos().y;
        vMax.x += ImGui::GetWindowPos().x;
        vMax.y += ImGui::GetWindowPos().y;

        m_wndPos.x = vMin.x;
        m_wndPos.y = vMin.y;

        m_wndContentAreaSize = Vec2(glm::abs(vMax.x - vMin.x), glm::abs(vMax.y - vMin.y));

        ImGuiIO& io = ImGui::GetIO();
        ImVec2 absMousePos = io.MousePos;
        m_mouseOverContentArea = false;
        if (vMin.x < absMousePos.x && vMax.x > absMousePos.x)
        {
          if (vMin.y < absMousePos.y && vMax.y > absMousePos.y)
          {
            m_mouseOverContentArea = true;
          }
        }

        m_lastMousePosRelContentArea.x = (int)(absMousePos.x - vMin.x);
        m_lastMousePosRelContentArea.y = (int)(absMousePos.y - vMin.y);

        if (!ImGui::IsWindowCollapsed())
        {
          if (m_wndContentAreaSize.x > 0 && m_wndContentAreaSize.y > 0)
          {
            ImGui::Image((void*)(intptr_t)m_viewportImage->m_textureId, ImVec2(m_width, m_height), ImVec2(0.0f, 0.0f), ImVec2(1.0f, -1.0f));

            if (m_wndContentAreaSize.x != m_width || m_wndContentAreaSize.y != m_height)
            {
              OnResize(m_wndContentAreaSize.x, m_wndContentAreaSize.y);
            }

            if (IsActive())
            {
              ImGui::GetWindowDrawList()->AddRect(vMin, vMax, IM_COL32(255, 255, 0, 255));
            }
            else
            {
              ImGui::GetWindowDrawList()->AddRect(vMin, vMax, IM_COL32(128, 128, 128, 255));
            }
          }
        }

        m_mouseHover = ImGui::IsWindowHovered();

        // Process draw commands.
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        for (auto command : m_drawCommands)
        {
          command(drawList);
        }
        m_drawCommands.clear();

        // AssetBrowser drop handling.
        if (ImGui::BeginDragDropTarget())
        {
          if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BrowserDragZone"))
          {
            IM_ASSERT(payload->DataSize == sizeof(DirectoryEntry));
            DirectoryEntry entry = *(const DirectoryEntry*)payload->Data;

            if (entry.m_ext == MESH)
            {
              String path = ConcatPaths({ entry.m_rootPath, entry.m_fileName + entry.m_ext });
              
              Drawable* dwMesh = new Drawable();
              if (io.KeyShift)
              {
                MeshPtr mesh = GetMeshManager()->Create<Mesh>(path);
                dwMesh->m_mesh = mesh->Copy<Mesh>();
              }
              else
              {
                dwMesh->m_mesh = GetMeshManager()->Create<Mesh>(path);
              }
              
              dwMesh->m_mesh->Init(false);
              Ray ray = RayFromMousePosition();
              Vec3 pos = PointOnRay(ray, 5.0f);
              g_app->m_grid->HitTest(ray, pos);
              dwMesh->m_node->SetTranslation(pos);
              g_app->m_scene->AddEntity(dwMesh);
              g_app->m_scene->AddToSelection(dwMesh->m_id, false);
              SetActive();
            }
            else if (entry.m_ext == SCENE)
            {
              YesNoWindow* importOptionWnd = new YesNoWindow("Open Scene", "Open", "Merge", "Open or merge the scene ?", true);
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

        if (g_app->m_showOverlayUI)
        {
          if (IsActive() || g_app->m_showOverlayUIAlways)
          {
            bool onPlugin = false;
            if (m_name == "Perspective" && g_app->m_gameMod != App::GameMod::Stop)
            {
              if (!g_app->m_runWindowed)
              {
                // Game is being drawn on perspective. Hide overlays.
                onPlugin = true;
              }
            }

            if (m_name == "PlayWindow")
            {
              onPlugin = true;
            }

            if (!onPlugin)
            {
              for (OverlayUI* overlay : m_overlays)
              {
                if (overlay)
                {
                  overlay->m_owner = this;
                  overlay->Show();
                }
              }
            }

          }
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

      PanZoom(deltaTime);
    }

    void EditorViewport2d::OnResize(float width, float height)
    {
      Viewport::OnResize(width, height);
      AdjustZoom(0.0f);
    }

    void EditorViewport2d::Init2dCam()
    {
      m_zoom = 1.0f;
      m_camera->m_node->SetTranslation(Z_AXIS * 10.0f);
      AdjustZoom(0.0f);
    }

    void EditorViewport2d::GetGlobalCanvasSize()
    {
      m_canvasSize =
      {
        g_app->m_playWidth,
        g_app->m_playHeight
      };
    }

    void EditorViewport2d::PanZoom(float deltaTime)
    {
      if (m_camera)
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
          m_camera->m_node->Translate(displace, TransformationSpace::TS_WORLD);
        }
      }
    }

    void EditorViewport2d::AdjustZoom(float z)
    {
      m_zoom += z * 0.1f;
      m_zoom = glm::max(0.1f, m_zoom);
      m_camera->SetLens
      (
        m_width / m_height,
        m_canvasSize.x * m_zoom * -0.5f,
        m_canvasSize.x * m_zoom * 0.5f,
        m_canvasSize.y * m_zoom * -0.5f,
        m_canvasSize.y * m_zoom * 0.5f,
        0.01f,
        1000.0f
      );
    }

  }
}
