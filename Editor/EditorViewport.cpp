#include "EditorViewport.h"

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
#include "DirectionComponent.h"
#include "DebugNew.h"
#include "FileManager.h"


namespace ToolKit
{
  namespace Editor
  {

    std::vector<OverlayUI*> EditorViewport::m_overlays =
    {
      nullptr,
      nullptr,
      nullptr,
      nullptr
    };

    void InitOverlays(EditorViewport* viewport)
    {
      for (int i = 0; i < 4; i++)
      {
        OverlayUI** overlay = &EditorViewport::m_overlays[i];
        if (*overlay == nullptr)
        {
          switch (i)
          {
          case 0:
            *overlay = new OverlayMods(viewport);
            break;
          case 1:
            *overlay = new OverlayViewportOptions(viewport);
            break;
          case 2:
            *overlay = new StatusBar(viewport);
            break;
          case 3:
            *overlay = new OverlayLighting(viewport);
            break;
          }
        }
      }
    }

    EditorViewport::EditorViewport(XmlNode* node)
    {
      DeSerialize(nullptr, node);
      ResetViewportImage(GetRenderTargetSettings());
      InitOverlays(this);
      m_snapDeltas = Vec3(0.25f, 45.0f, 0.25f);
    }

    EditorViewport::EditorViewport(float width, float height)
      : Viewport(width, height)
    {
      m_name = g_viewportStr + " " + std::to_string(m_id);
      InitOverlays(this);
      m_snapDeltas = Vec3(0.25f, 45.0f, 0.25f);
    }

    EditorViewport::~EditorViewport()
    {
    }

    void EditorViewport::Show()
    {
      m_mouseOverOverlay = false;

      ImGui::SetNextWindowSize(ImVec2(m_width, m_height), ImGuiCond_None);

      if
      (
        ImGui::Begin
        (
          m_name.c_str(),
          &m_visible,
          ImGuiWindowFlags_NoScrollbar
          | ImGuiWindowFlags_NoScrollWithMouse
          | m_additionalWindowFlags
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

    void EditorViewport::Update(float deltaTime)
    {
      if (!IsActive())
      {
        SDL_GetGlobalMouseState(&m_mousePosBegin.x, &m_mousePosBegin.y);
        return;
      }

      // Update viewport mods.
      FpsNavigationMode(deltaTime);
      OrbitPanMod(deltaTime);
    }

    Window::Type EditorViewport::GetType() const
    {
      return Type::Viewport;
    }

    bool EditorViewport::IsViewportQueriable() const
    {
      return m_mouseOverContentArea
        && m_mouseHover
        && m_active
        && m_visible
        && m_relMouseModBegin;
    }

    void EditorViewport::DispatchSignals() const
    {
      if (!CanDispatchSignals() || m_mouseOverOverlay)
      {
        return;
      }

      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      {
        ModManager::GetInstance()->DispatchSignal
        (
          BaseMod::m_leftMouseBtnDownSgnl
        );
      }

      if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
      {
        ModManager::GetInstance()->DispatchSignal
        (
          BaseMod::m_leftMouseBtnUpSgnl
        );
      }

      if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
      {
        ModManager::GetInstance()->DispatchSignal
        (
          BaseMod::m_leftMouseBtnDragSgnl
        );
      }

      if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
      {
        ModManager::GetInstance()->DispatchSignal(BaseMod::m_delete);
      }

      ModShortCutSignals();
    }

    void EditorViewport::Serialize(XmlDocument* doc, XmlNode* parent) const
    {
      Window::Serialize(doc, parent);
      XmlNode* node = doc->allocate_node(rapidxml::node_element, "Viewport");

      WriteAttr(node, doc, "width", std::to_string(m_width));
      WriteAttr(node, doc, "height", std::to_string(m_height));
      WriteAttr
      (
        node,
        doc,
        "alignment",
        std::to_string(static_cast<int>(m_cameraAlignment))
      );
      WriteAttr
      (
        node,
        doc,
        "lock",
        std::to_string(static_cast<int>(m_orbitLock))
      );
      GetCamera()->Serialize(doc, node);

      XmlNode* wnd = parent->last_node();
      wnd->append_node(node);
    }

    void EditorViewport::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      Window::DeSerialize(doc, parent);

      if (XmlNode* node = parent->first_node("Viewport"))
      {
        ReadAttr(node, "width", m_width);
        ReadAttr(node, "height", m_height);
        ReadAttr
        (
          node,
          "alignment",
          *(reinterpret_cast<int*>(&m_cameraAlignment))
        );
        ReadAttr(node, "lock", m_orbitLock);
        SetCamera(new Camera(node->first_node("E")));
      }
    }

    void EditorViewport::OnResize(float width, float height)
    {
      Viewport::OnResize(width, height);
      AdjustZoom(0.0f);
    }

    void EditorViewport::GetContentAreaScreenCoordinates
    (
      Vec2* min,
      Vec2* max
    ) const
    {
      *min = m_wndPos;
      *max = m_wndPos + m_wndContentAreaSize;
    }

    void EditorViewport::SetCamera(Camera* cam)
    {
      Viewport::SetCamera(cam);
      AdjustZoom(0.0f);
    }

    RenderTargetSettigs EditorViewport::GetRenderTargetSettings()
    {
      RenderTargetSettigs sets;
      sets.Msaa = 8;
      return sets;
    }

    void EditorViewport::UpdateContentArea()
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
      ImGuiIO& io = ImGui::GetIO();
      ImVec2 absMousePos = io.MousePos;
      m_mouseOverContentArea = false;
      if
      (
        m_contentAreaMin.x < absMousePos.x
        && m_contentAreaMax.x > absMousePos.x
      )
      {
        if
        (
          m_contentAreaMin.y < absMousePos.y
          && m_contentAreaMax.y > absMousePos.y
        )
        {
          m_mouseOverContentArea = true;
        }
      }

      m_lastMousePosRelContentArea.x = static_cast<int>
      (
        absMousePos.x - m_contentAreaMin.x
      );
      m_lastMousePosRelContentArea.y = static_cast<int>
      (
        absMousePos.y - m_contentAreaMin.y
      );
    }

    void EditorViewport::UpdateWindow()
    {
      if (!ImGui::IsWindowCollapsed())
      {
        if (m_wndContentAreaSize.x > 0 && m_wndContentAreaSize.y > 0)
        {
          ImGui::Image
          (
            Convert2ImGuiTexture(m_viewportImage),
            Vec2(m_width, m_height),
            Vec2(0.0f, 0.0f),
            Vec2(1.0f, -1.0f)
          );

          if
          (
            m_wndContentAreaSize.x != m_width ||
            m_wndContentAreaSize.y != m_height
          )
          {
            OnResize(m_wndContentAreaSize.x, m_wndContentAreaSize.y);
          }

          if (IsActive())
          {
            ImGui::GetWindowDrawList()->AddRect
            (
              m_contentAreaMin,
              m_contentAreaMax,
              IM_COL32(255, 255, 0, 255)
            );
          }
          else
          {
            ImGui::GetWindowDrawList()->AddRect
            (
              m_contentAreaMin,
              m_contentAreaMax,
              IM_COL32(128, 128, 128, 255)
            );
          }
        }
      }

      m_mouseHover = ImGui::IsWindowHovered();
    }

    void EditorViewport::DrawCommands()
    {
      // Process draw commands.
      ImDrawList* drawList = ImGui::GetWindowDrawList();
      for (auto command : m_drawCommands)
      {
        command(drawList);
      }
      m_drawCommands.clear();
    }

    void EditorViewport::FpsNavigationMode(float deltaTime)
    {
      Camera* cam = GetCamera();
      if (cam && !cam->IsOrtographic())
      {
        // Mouse is right clicked
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
          ImGui::SetMouseCursor(ImGuiMouseCursor_None);

          // Handle relative mouse hack.
          if (m_relMouseModBegin)
          {
            m_relMouseModBegin = false;
            SDL_GetGlobalMouseState(&m_mousePosBegin.x, &m_mousePosBegin.y);
          }

          IVec2 absMousePos;
          SDL_GetGlobalMouseState(&absMousePos.x, &absMousePos.y);
          IVec2 delta = absMousePos - m_mousePosBegin;

          SDL_WarpMouseGlobal(m_mousePosBegin.x, m_mousePosBegin.y);
          // End of relative mouse hack.

          if (!VecAllEqual<IVec2>(delta, glm::zero<IVec2>()))
          {
            if (m_cameraAlignment != CameraAlignment::User)
            {
              m_cameraAlignment = CameraAlignment::Free;
            }
          }

          cam->GetComponent<DirectionComponent>()->Pitch
          (
            -glm::radians(delta.y * g_app->m_mouseSensitivity)
          );
          cam->GetComponent<DirectionComponent>()->RotateOnUpVector
          (
            -glm::radians(delta.x * g_app->m_mouseSensitivity)
          );

          Vec3 dir, up, right;
          dir = -Z_AXIS;
          up = Y_AXIS;
          right = X_AXIS;

          float speed = g_app->m_camSpeed;

          Vec3 move;
          if (ImGui::IsKeyDown(ImGuiKey_A))
          {
            move += -right;
          }

          if (ImGui::IsKeyDown(ImGuiKey_D))
          {
            move += right;
          }

          if (ImGui::IsKeyDown(ImGuiKey_W))
          {
            move += dir;
          }

          if (ImGui::IsKeyDown(ImGuiKey_S))
          {
            move += -dir;
          }

          if (ImGui::IsKeyDown(ImGuiKey_PageUp))
          {
            move += up;
          }

          if (ImGui::IsKeyDown(ImGuiKey_PageDown))
          {
            move += -up;
          }

          float displace = speed * MillisecToSec(deltaTime);
          if (length(move) > 0.0f)
          {
            move = normalize(move);
          }

          cam->m_node->Translate
          (
            move * displace,
            TransformationSpace::TS_LOCAL
          );
        }
        else
        {
          if (!m_relMouseModBegin)
          {
            m_relMouseModBegin = true;
          }
        }
      }
    }

    void EditorViewport::OrbitPanMod(float deltaTime)
    {
      Camera* cam = GetCamera();
      if (cam)
      {
        // Adjust zoom always.
        ImGuiIO& io = ImGui::GetIO();
        if (m_mouseOverContentArea)
        {
          float delta = io.MouseWheel;
          if (glm::notEqual<float>(delta, 0.0f))
          {
            AdjustZoom(delta);
          }
        }

        static Vec3 orbitPnt;
        static bool hitFound = false;
        static float dist = 0.0f;
        Camera::CamData dat = cam->GetData();
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
          // Figure out orbiting point.
          EditorScenePtr currScene = g_app->GetCurrentScene();
          Entity* currEntity = currScene->GetCurrentSelection();
          if (currEntity == nullptr)
          {
            if (!hitFound)
            {
              Ray orbitRay = RayFromMousePosition();
              EditorScene::PickData pd = currScene->PickObject(orbitRay);

              if (pd.entity == nullptr)
              {
                if (!g_app->m_grid->HitTest(orbitRay, orbitPnt))
                {
                  orbitPnt = PointOnRay(orbitRay, 5.0f);
                }
              }
              else
              {
                orbitPnt = pd.pickPos;
              }
              hitFound = true;
              dist = glm::distance(orbitPnt, dat.pos);
            }
          }
          else
          {
            hitFound = true;
            orbitPnt = currEntity->m_node->GetTranslation
            (
              TransformationSpace::TS_WORLD
            );
            dist = glm::distance(orbitPnt, dat.pos);
          }

          // Orbit around it.
          float x = io.MouseDelta.x;
          float y = io.MouseDelta.y;
          Vec3 r = cam->GetComponent<DirectionComponent>()->GetRight();
          Vec3 u = cam->GetComponent<DirectionComponent>()->GetUp();

          if (io.KeyShift || m_orbitLock)
          {
            // Reflect window space mouse delta to image plane.
            Vec3 deltaOnImagePlane = glm::unProject
            (
              // Here, mouse delta is transformed to viewport center.
              Vec3(x + m_width * 0.5f, y + m_height * 0.5f, 0.0f),
              Mat4(),
              dat.projection,
              Vec4(0.0f, 0.0f, m_width, m_height)
            );

            // Thales ! Reflect imageplane displacement to world space.
            Vec3 deltaOnWorld = deltaOnImagePlane * dist / dat.nearDist;
            if (cam->IsOrtographic())
            {
              deltaOnWorld = deltaOnImagePlane;
            }

            Vec3 displace = r * -deltaOnWorld.x + u * deltaOnWorld.y;
            cam->m_node->Translate(displace, TransformationSpace::TS_WORLD);
          }
          else
          {
            if (m_cameraAlignment != CameraAlignment::Free)
            {
              if (m_cameraAlignment == CameraAlignment::Top)
              {
                orbitPnt.y = 0.0f;
              }
              else if (m_cameraAlignment == CameraAlignment::Front)
              {
                orbitPnt.z = 0.0f;
              }
              else if (m_cameraAlignment == CameraAlignment::Left)
              {
                orbitPnt.x = 0.0f;
              }
            }

            Mat4 camTs = cam->m_node->GetTransform
            (
              TransformationSpace::TS_WORLD
            );
            Mat4 ts = glm::translate(Mat4(), orbitPnt);
            Mat4 its = glm::translate(Mat4(), -orbitPnt);
            Quaternion qx = glm::angleAxis
            (
              -glm::radians(y * g_app->m_mouseSensitivity),
              r
            );
            Quaternion qy = glm::angleAxis
            (
              -glm::radians(x * g_app->m_mouseSensitivity),
              Y_AXIS
            );

            camTs = ts * glm::toMat4(qy * qx) * its * camTs;
            cam->m_node->SetTransform(camTs, TransformationSpace::TS_WORLD);

            if (m_cameraAlignment != CameraAlignment::User)
            {
              m_cameraAlignment = CameraAlignment::Free;
            }
          }
        }

        if (!ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        {
          hitFound = false;
          dist = 0.0f;
        }
      }
    }

    void EditorViewport::AdjustZoom(float delta)
    {
      Camera* cam = GetCamera();
      cam->m_node->Translate
      (
        Vec3(0.0f, 0.0f, -delta),
        TransformationSpace::TS_LOCAL
      );
      if (cam->IsOrtographic())
      {
        // Magic zoom.
        Camera::CamData dat = cam->GetData();
        float dist = glm::distance(ZERO, dat.pos);
        m_zoom = dist / 600.0f;
        cam->SetLens
        (
          -m_zoom * m_width * 0.5f,
          m_zoom * m_width * 0.5f,
          -m_zoom * m_height * 0.5f,
          m_zoom * m_height * 0.5f,
          0.5f,
          1000.0f
        );
      }
    }

    void EditorViewport::HandleDrop()
    {
      ImGuiIO& io = ImGui::GetIO();

      // Current scene
      EditorScenePtr currScene = g_app->GetCurrentScene();

      // Asset drag and drop loading variables
      static LineBatch* boundingBox = nullptr;
      static bool meshLoaded = false;
      static bool meshAddedToScene = false;
      static Entity* dwMesh = nullptr;

      // AssetBrowser drop handling.
      if (ImGui::BeginDragDropTarget())
      {
        const ImGuiPayload* dragPayload = ImGui::GetDragDropPayload();
        if (dragPayload->DataSize != sizeof(DirectoryEntry))
        {
          return;
        }
        DirectoryEntry dragEntry = *(const DirectoryEntry*)dragPayload->Data;

        // Check if the drag object is a mesh
        Vec3 lastDragMeshPos = Vec3(0.0f);
        if (dragEntry.m_ext == MESH || dragEntry.m_ext == SKINMESH)
        {
          // Load mesh
          LoadDragMesh
          (
            meshLoaded,
            dragEntry,
            io,
            &dwMesh,
            &boundingBox,
            currScene
          );

          // Show bounding box
          lastDragMeshPos = CalculateDragMeshPosition
          (
            meshLoaded,
            currScene,
            dwMesh,
            &boundingBox
          );
        }

        if
        (
          const ImGuiPayload* payload = ImGui::AcceptDragDropPayload
          (
            "BrowserDragZone"
          )
        )
        {
          IM_ASSERT(payload->DataSize == sizeof(DirectoryEntry));
          DirectoryEntry entry = *(const DirectoryEntry*)payload->Data;

          if (entry.m_ext == MESH || entry.m_ext == SKINMESH)
          {
            // Translate mesh to correct position
            dwMesh->m_node->SetTranslation(
              lastDragMeshPos,
              TransformationSpace::TS_WORLD
            );

            // Add mesh to the scene
            currScene->AddEntity(dwMesh);
            currScene->AddToSelection(dwMesh->GetIdVal(), false);
            SetActive();

            meshAddedToScene = true;
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
          else if (entry.m_ext == MATERIAL)
          {
            // Find the drop entity
            Ray ray = RayFromMousePosition();
            EditorScene::PickData pd = currScene->PickObject(ray);
            if (pd.entity != nullptr && pd.entity->IsDrawable())
            {
              MeshComponentPtr ms = pd.entity->GetComponent<MeshComponent>();
              if (ms != nullptr)
              {
                // Load material once
                String path = ConcatPaths
                (
                  {
                    dragEntry.m_rootPath,
                    dragEntry.m_fileName + dragEntry.m_ext
                  }
                );
                MaterialPtr material = GetMaterialManager()->Create<Material>
                (
                  path
                );
                // Set material to material component
                MaterialComponentPtr matPtr =
                pd.entity->GetMaterialComponent();
                if (matPtr == nullptr)
                {
                  // Create a new material component
                  MaterialComponent* matComp = new MaterialComponent();
                  pd.entity->AddComponent(matComp);
                  matPtr = pd.entity->GetMaterialComponent();
                }
                matPtr->m_localData[matPtr->MaterialIndex()] = material;
              }
            }
          }
        }

        ImGui::EndDragDropTarget();
      }

      HandleDropMesh
      (
        meshLoaded,
        meshAddedToScene,
        currScene,
        &dwMesh,
        &boundingBox
      );
    }

    void EditorViewport::DrawOverlays()
    {
      if (g_app->m_showOverlayUI)
      {
        if (IsActive() || g_app->m_showOverlayUIAlways)
        {
          bool onPlugin = false;
          if (m_name == g_3dViewport && g_app->m_gameMod != App::GameMod::Stop)
          {
            if (!g_app->m_emulatorSettings.runWindowed)
            {
              // Game is being drawn on 3d viewport. Hide overlays.
              onPlugin = true;
            }
          }

          if (m_name == g_simulationViewport)
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

    void EditorViewport::LoadDragMesh
    (
      bool& meshLoaded,
      DirectoryEntry dragEntry,
      ImGuiIO io,
      Entity** dwMesh,
      LineBatch** boundingBox,
      EditorScenePtr currScene
    )
    {
      if (!meshLoaded)
      {
        // Load mesh once
        String path = ConcatPaths
        (
          { dragEntry.m_rootPath, dragEntry.m_fileName + dragEntry.m_ext }
        );
        *dwMesh = new Entity();
        (*dwMesh)->AddComponent(new MeshComponent);
        MeshPtr mesh;
        if (dragEntry.m_ext == SKINMESH)
        {
          mesh = GetMeshManager()->Create<SkinMesh>(path);
        }
        else
        {
          mesh = GetMeshManager()->Create<Mesh>(path);
        }
        (*dwMesh)->GetMeshComponent()->SetMeshVal(mesh);
        mesh->Init(false);

        // Load bounding box once
        *boundingBox = CreateBoundingBoxDebugObject((*dwMesh)->GetAABB(true));

        // Add bounding box to the scene
        currScene->AddEntity(*boundingBox);

        meshLoaded = true;
      }
    }

    Vec3 EditorViewport::CalculateDragMeshPosition
    (
      bool& meshLoaded,
      EditorScenePtr currScene,
      Entity* dwMesh,
      LineBatch** boundingBox
    )
    {
      Vec3 lastDragMeshPos = Vec3(0.0f);

      // Find the point of the cursor in 3D coordinates
      Ray ray = RayFromMousePosition();
      EntityIdArray ignoreList;
      if (meshLoaded)
      {
        ignoreList.push_back((*boundingBox)->GetIdVal());
      }
      EditorScene::PickData pd = currScene->PickObject(ray, ignoreList);
      bool meshFound = false;
      if (pd.entity != nullptr)
      {
        meshFound = true;
        lastDragMeshPos = pd.pickPos;
      }
      else
      {
        // Locate the mesh to grid
        lastDragMeshPos = PointOnRay(ray, 5.0f);
        g_app->m_grid->HitTest(ray, lastDragMeshPos);
      }

      // Change drop mode with space key
      static bool boxMode = false;
      if (ImGui::IsKeyPressed(ImGuiKey_Space, false))
      {
        boxMode = !boxMode;
      }

      if (meshFound && boxMode)
      {
        float firstY = lastDragMeshPos.y;
        lastDragMeshPos.y -= dwMesh->GetAABB(false).min.y;

        if (firstY > lastDragMeshPos.y)
        {
          lastDragMeshPos.y = firstY;
        }
      }

      (*boundingBox)->m_node->SetTranslation
      (
        lastDragMeshPos,
        TransformationSpace::TS_WORLD
      );

      return lastDragMeshPos;
    }

    void EditorViewport::HandleDropMesh
    (
      bool& meshLoaded,
      bool& meshAddedToScene,
      EditorScenePtr currScene,
      Entity** dwMesh,
      LineBatch** boundingBox
    )
    {
      if (meshLoaded && !ImGui::IsMouseDragging(0))
      {
        // Remove debug bounding box mesh from scene
        currScene->RemoveEntity((*boundingBox)->GetIdVal());
        meshLoaded = false;

        if (!meshAddedToScene)
        {
          SafeDel(*dwMesh);
        }
        else
        {
          meshAddedToScene = false;
        }

        // Unload bounding box mesh
        SafeDel(*boundingBox);
      }
    }
  }  // namespace Editor
}  // namespace ToolKit
