/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "EditorViewport.h"

#include "App.h"
#include "LeftBar.h"
#include "Mod.h"
#include "OverlayLighting.h"
#include "PopupWindows.h"
#include "StatusBar.h"
#include "TopBar.h"

#include <Camera.h>
#include <DirectionComponent.h>
#include <Material.h>
#include <MathUtil.h>
#include <Mesh.h>
#include <MeshComponent.h>
#include <Prefab.h>
#include <SDL.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {

    TKDefineClass(EditorViewport, Window);

    std::vector<OverlayUI*> EditorViewport::m_overlays = {nullptr, nullptr, nullptr, nullptr};

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
            *overlay = new OverlayLeftBar(viewport);
            break;
          case 1:
            *overlay = new OverlayTopBar(viewport);
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

    EditorViewport::EditorViewport()
    {
      m_name = g_viewportStr + " " + std::to_string(m_id);
      Init({640.0f, 480.0f});

      m_editorRenderer = MakeNewPtr<EditorRenderer>();
    }

    EditorViewport::~EditorViewport() {}

    void EditorViewport::Show()
    {
      m_mouseOverOverlay = false;
      ImGui::SetNextWindowSize(Vec2(m_size), ImGuiCond_None);
      ImGui::PushStyleColor(ImGuiCol_WindowBg, g_wndBgColor);

      if (ImGui::Begin(m_name.c_str(),
                       &m_visible,
                       ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | m_additionalWindowFlags))
      {
        UpdateContentArea();
        ComitResize();
        UpdateWindow();
        HandleStates();
        HandleDrop();
        DrawOverlays();
        DrawCommands();
        UpdateSnaps();
      }

      ImGui::End();
      ImGui::PopStyleColor();
    }

    void EditorViewport::Update(float deltaTime)
    {
      if (!IsActive())
      {
        SDL_GetGlobalMouseState(&m_mousePosBegin.x, &m_mousePosBegin.y);
        return;
      }

      // Update viewport mods.
      FpsNavigationMod(deltaTime);
      OrbitPanMod(deltaTime);
    }

    bool EditorViewport::IsViewportQueriable() const
    {
      return m_mouseOverContentArea && m_mouseHover && m_active && m_visible && m_relMouseModBegin;
    }

    void EditorViewport::DispatchSignals() const
    {
      if (!CanDispatchSignals() || m_mouseOverOverlay)
      {
        return;
      }

      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      {
        ModManager::GetInstance()->DispatchSignal(BaseMod::m_leftMouseBtnDownSgnl);
      }

      if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
      {
        ModManager::GetInstance()->DispatchSignal(BaseMod::m_leftMouseBtnUpSgnl);
      }

      if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
      {
        ModManager::GetInstance()->DispatchSignal(BaseMod::m_leftMouseBtnDragSgnl);
      }

      if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
      {
        ModManager::GetInstance()->DispatchSignal(BaseMod::m_delete);
      }

      ModShortCutSignals();
    }

    XmlNode* EditorViewport::SerializeImp(XmlDocument* doc, XmlNode* parent) const
    {
      XmlNode* wndNode = Super::SerializeImp(doc, parent);
      XmlNode* node    = CreateXmlNode(doc, "Viewport", wndNode);

      WriteAttr(node, doc, "alignment", std::to_string((int) m_cameraAlignment));
      WriteAttr(node, doc, "lock", std::to_string((int) m_orbitLock));
      GetCamera()->Serialize(doc, node);

      return node;
    }

    XmlNode* EditorViewport::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
    {
      XmlNode* wndNode      = Super::DeSerializeImp(info, parent);
      XmlNode* viewportNode = wndNode->first_node("Viewport");
      m_wndContentAreaSize  = m_size;

      if (viewportNode)
      {
        ReadAttr(viewportNode, "alignment", *((int*) (&m_cameraAlignment)));
        ReadAttr(viewportNode, "lock", m_orbitLock);

        CameraPtr viewCam  = MakeNewPtr<Camera>();
        viewCam->m_version = m_version;
        ULongID id         = viewCam->GetIdVal();

        if (m_version > TKV044)
        {
          XmlNode* objNode = viewportNode->first_node(Object::StaticClass()->Name.c_str());
          viewCam->DeSerialize(info, objNode);
        }
        else
        {
          viewCam->DeSerialize(info, viewportNode->first_node("E"));
        }
        viewCam->SetIdVal(id);

        // Reset aspect.
        if (!viewCam->IsOrtographic())
        {
          viewCam->SetLens(glm::quarter_pi<float>(), viewCam->Aspect());
        }

        SetCamera(viewCam);
      }

      return viewportNode;
    }

    void EditorViewport::OnResizeContentArea(float width, float height)
    {
      Viewport::OnResizeContentArea(width, height);
      AdjustZoom(0.0f);
    }

    void EditorViewport::ResizeWindow(uint width, uint height)
    {
      m_size.x      = width;
      m_size.y      = height;
      m_needsResize = true;
    }

    void EditorViewport::GetContentAreaScreenCoordinates(Vec2* min, Vec2* max) const
    {
      *min = m_contentAreaLocation;
      *max = m_contentAreaLocation + m_wndContentAreaSize;
    }

    void EditorViewport::SetCamera(CameraPtr cam)
    {
      Viewport::SetCamera(cam);
      AdjustZoom(0.0f);
    }

    void EditorViewport::UpdateContentArea()
    {
      // Content area size

      m_contentAreaMin       = ImGui::GetWindowContentRegionMin();
      m_contentAreaMax       = ImGui::GetWindowContentRegionMax();

      Vec2 wndPos            = Vec2(ImGui::GetWindowPos());
      m_contentAreaMin      += wndPos;
      m_contentAreaMax      += wndPos;

      m_contentAreaLocation  = m_contentAreaMin;

      const Vec2 prevSize    = m_wndContentAreaSize;

      m_wndContentAreaSize   = glm::abs(m_contentAreaMax - m_contentAreaMin);

      if (glm::all(glm::epsilonNotEqual(prevSize, m_wndContentAreaSize, 0.001f)))
      {
        m_needsResize = true;
      }

      ImGuiIO& io            = ImGui::GetIO();
      Vec2 absMousePos       = io.MousePos;

      m_mouseOverContentArea = false;

      if (m_contentAreaMin.x < absMousePos.x && m_contentAreaMax.x > absMousePos.x)
      {
        if (m_contentAreaMin.y < absMousePos.y && m_contentAreaMax.y > absMousePos.y)
        {
          m_mouseOverContentArea = true;
        }
      }

      m_lastMousePosRelContentArea = absMousePos - m_contentAreaMin;
    }

    void EditorViewport::UpdateWindow()
    {
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
          uint texId = 0;
          if (m_framebuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0) != nullptr)
          {
            texId = m_framebuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0)->m_textureId;
          }

          ImDrawList* drawList = ImGui::GetWindowDrawList();
          drawList->AddCallback([](const ImDrawList* parentList, const ImDrawCmd* cmd)
                                { GetRenderSystem()->EnableBlending(false); },
                                nullptr);

          ImGui::Image(ConvertUIntImGuiTexture(texId), m_wndContentAreaSize, Vec2(0.0f, 0.0f), Vec2(1.0f, -1.0f));

          drawList->AddCallback([](const ImDrawList* parentList, const ImDrawCmd* cmd)
                                { GetRenderSystem()->EnableBlending(true); },
                                nullptr);

          if (IsActive())
          {
            ImGui::GetWindowDrawList()->AddRect(m_contentAreaMin, m_contentAreaMax, IM_COL32(255, 255, 0, 255));
          }
          else
          {
            ImGui::GetWindowDrawList()->AddRect(m_contentAreaMin, m_contentAreaMax, IM_COL32(128, 128, 128, 255));
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

    void EditorViewport::FpsNavigationMod(float deltaTime)
    {
      CameraPtr cam = GetCamera();
      if (cam == nullptr)
      {
        return;
      }

      // Allow user camera to fps navigate even in orthographic mod.
      if (m_attachedCamera != NULL_HANDLE || !cam->IsOrtographic())
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

          cam->GetComponent<DirectionComponent>()->Pitch(-glm::radians(delta.y * g_app->m_mouseSensitivity));
          cam->GetComponent<DirectionComponent>()->RotateOnUpVector(-glm::radians(delta.x * g_app->m_mouseSensitivity));

          Vec3 dir, up, right;
          dir         = -Z_AXIS;
          up          = Y_AXIS;
          right       = X_AXIS;

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

          cam->m_node->Translate(move * displace, TransformationSpace::TS_LOCAL);
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
      CameraPtr cam = GetCamera();
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
        static float dist    = 0.0f;
        const Vec3 camPos    = cam->m_node->GetTranslation();
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
          // Figure out orbiting point.
          EditorScenePtr currScene = g_app->GetCurrentScene();
          EntityPtr currEntity     = currScene->GetCurrentSelection();
          if (currEntity == nullptr)
          {
            if (!hitFound)
            {
              Ray orbitRay             = RayFromMousePosition();
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
              dist     = glm::distance(orbitPnt, camPos);
            }
          }
          else
          {
            hitFound = true;
            orbitPnt = currEntity->m_node->GetTranslation(TransformationSpace::TS_WORLD);
            dist     = glm::distance(orbitPnt, camPos);
          }

          // Orbit around it.
          float x = io.MouseDelta.x;
          float y = io.MouseDelta.y;
          Vec3 r  = cam->GetComponent<DirectionComponent>()->GetRight();
          Vec3 u  = cam->GetComponent<DirectionComponent>()->GetUp();

          if (io.KeyShift || m_orbitLock)
          {
            // Reflect window space mouse delta to image plane.
            Vec3 deltaOnImagePlane = glm::unProject(
                // Here, mouse delta is transformed to viewport center.
                Vec3(x + m_wndContentAreaSize.x * 0.5f, y + m_wndContentAreaSize.y * 0.5f, 0.0f),
                Mat4(),
                cam->GetProjectionMatrix(),
                Vec4(0.0f, 0.0f, m_wndContentAreaSize.x, m_wndContentAreaSize.y));

            // Thales ! Reflect imageplane displacement to world space.
            Vec3 deltaOnWorld = deltaOnImagePlane * dist / cam->Near();
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

            Mat4 camTs    = cam->m_node->GetTransform(TransformationSpace::TS_WORLD);
            Mat4 ts       = glm::translate(Mat4(), orbitPnt);
            Mat4 its      = glm::translate(Mat4(), -orbitPnt);
            Quaternion qx = glm::angleAxis(-glm::radians(y * g_app->m_mouseSensitivity), r);
            Quaternion qy = glm::angleAxis(-glm::radians(x * g_app->m_mouseSensitivity), Y_AXIS);

            camTs         = ts * glm::toMat4(qy * qx) * its * camTs;
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
          dist     = 0.0f;
        }
      }
    }

    void EditorViewport::AdjustZoom(float delta)
    {
      CameraPtr cam = GetCamera();
      cam->m_node->Translate(Vec3(0.0f, 0.0f, -delta), TransformationSpace::TS_LOCAL);

      if (cam->IsOrtographic())
      {
        // Don't allow user camera to have magic zoom.
        if (m_attachedCamera == NULL_HANDLE)
        {
          // Magic zoom.
          const Vec3 camPos        = cam->m_node->GetTranslation();
          float dist               = glm::distance(ZERO, camPos);
          cam->m_orthographicScale = dist / 600.0f;
        }
      }
    }

    void EditorViewport::HandleDrop()
    {
      // Current scene
      EditorScenePtr currScene        = g_app->GetCurrentScene();

      // Asset drag and drop loading variables
      static LineBatchPtr boundingBox = nullptr;
      static bool meshLoaded          = false;
      static bool meshAddedToScene    = false;
      static EntityPtr dwMesh         = nullptr;

      // AssetBrowser drop handling.
      if (ImGui::BeginDragDropTarget())
      {
        const ImGuiPayload* dragPayload = ImGui::GetDragDropPayload();
        if (dragPayload->DataSize != sizeof(FileDragData))
        {
          return;
        }
        const FileDragData& dragData = FolderView::GetFileDragData();
        DirectoryEntry& entry        = *dragData.Entries[0]; // get first entry

        // Check if the drag object is a mesh
        Vec3 lastDragMeshPos         = Vec3(0.0f);
        if (entry.m_ext == MESH || entry.m_ext == SKINMESH)
        {
          // Load mesh
          LoadDragMesh(meshLoaded, entry, &dwMesh, &boundingBox, currScene);

          // Show bounding box
          lastDragMeshPos = CalculateDragMeshPosition(meshLoaded, currScene, dwMesh, &boundingBox);
        }
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BrowserDragZone"))
        {

          if (entry.m_ext == MESH || entry.m_ext == SKINMESH)
          {
            // Translate mesh to correct position
            dwMesh->m_node->SetTranslation(lastDragMeshPos, TransformationSpace::TS_WORLD);

            if (entry.m_ext == SKINMESH)
            {
              if (dwMesh->GetComponent<AABBOverrideComponent>() == nullptr)
              {
                AABBOverrideComponentPtr aabbOverride = MakeNewPtr<AABBOverrideComponent>();
                aabbOverride->SetBoundingBox(dwMesh->GetBoundingBox());
                dwMesh->AddComponent(aabbOverride);
              }
            }

            // Add mesh to the scene
            currScene->AddEntity(dwMesh);
            currScene->AddToSelection(dwMesh->GetIdVal(), false);
            SetActive();

            meshAddedToScene = true;
          }
          else if (entry.m_ext == SCENE || entry.m_ext == LAYER)
          {
            MultiChoiceButtonInfo openButton;
            openButton.m_name     = "Open";
            openButton.m_callback = [entry]() -> void
            {
              String fullPath = entry.GetFullPath();
              g_app->OpenScene(fullPath);
            };

            MultiChoiceButtonInfo linkButton;
            linkButton.m_name     = "Link";
            linkButton.m_callback = [entry]() -> void
            {
              String fullPath = entry.GetFullPath();
              g_app->LinkScene(fullPath);
            };

            MultiChoiceButtonInfo mergeButton;
            mergeButton.m_name     = "Merge";
            mergeButton.m_callback = [entry]() -> void
            {
              String fullPath = entry.GetFullPath();
              g_app->MergeScene(fullPath);
            };

            MultiChoiceButtonArray buttons = {openButton, linkButton, mergeButton};
            MultiChoiceWindowPtr importOptionWnd =
                MakeNewPtr<MultiChoiceWindow>("Open Scene", buttons, "Open, link or merge the scene?", true);

            importOptionWnd->AddToUI();
          }
          else if (entry.m_ext == MATERIAL)
          {
            // Find the drop entity
            Ray ray                  = RayFromMousePosition();
            EditorScene::PickData pd = currScene->PickObject(ray);
            if (pd.entity != nullptr && pd.entity->IsDrawable())
            {
              // If there is a mesh component, update material component.
              if (Prefab::GetPrefabRoot(pd.entity) != nullptr)
              {
                g_app->m_statusMsg = "Failed. Target is Prefab.";
              }
              else
              {
                MeshRawPtrArray meshes;
                if (MeshComponentPtr meshComp = pd.entity->GetComponent<MeshComponent>())
                {
                  if (MeshPtr mesh = meshComp->GetMeshVal())
                  {
                    mesh->GetAllMeshes(meshes);
                  }
                }

                if (meshes.empty())
                {
                  return;
                }

                // Load material once
                String path                = ConcatPaths({entry.m_rootPath, entry.m_fileName + entry.m_ext});
                MaterialPtr material       = GetMaterialManager()->Create<Material>(path);

                // Create a material component if missing one.
                MaterialComponentPtr mmPtr = pd.entity->GetMaterialComponent();
                if (mmPtr == nullptr)
                {
                  g_app->m_statusMsg = "MaterialComponent added.";
                  mmPtr              = pd.entity->AddComponent<MaterialComponent>();
                  mmPtr->UpdateMaterialList();
                }

                // In case of submeshes exist, find sub mesh index.
                if (meshes.size() > 1)
                {
                  float t          = TK_FLT_MAX;
                  uint submeshIndx = FindMeshIntersection(pd.entity, ray, t);
                  if (submeshIndx != TK_UINT_MAX && t != TK_FLT_MAX)
                  {
                    mmPtr->GetMaterialList()[submeshIndx] = material;
                  }
                }
                else
                {
                  mmPtr->SetFirstMaterial(material);
                }
              }
            }
          }
        }

        ImGui::EndDragDropTarget();
      }

      HandleDropMesh(meshLoaded, meshAddedToScene, currScene, &dwMesh, &boundingBox);
    }

    void EditorViewport::DrawOverlays()
    {
      if (g_app->m_showOverlayUI)
      {
        if (IsActive() || g_app->m_showOverlayUIAlways)
        {
          bool onPlugin = false;
          if (m_name == g_3dViewport && g_app->m_gameMod != GameMod::Stop)
          {
            if (!g_app->m_simulatorSettings.Windowed)
            {
              // Game is being drawn on 3d viewport. Hide overlays.
              onPlugin = true;
            }
          }

          if (m_name == g_simulationViewStr)
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

    void EditorViewport::ComitResize()
    {
      if (m_needsResize)
      {
        Vec2 size(m_size);
        Vec2 windowStyleArea = size - m_wndContentAreaSize;
        Vec2 contentAreaSize = size - windowStyleArea;

        if (VecAllEqual(contentAreaSize, Vec2(0.0f)))
        {
          contentAreaSize = size;
        }

        OnResizeContentArea(contentAreaSize.x, contentAreaSize.y);
      }

      m_needsResize = false;
    }

    void EditorViewport::UpdateSnaps()
    {
      if (m_mouseOverContentArea && g_app->m_snapsEnabled)
      {
        g_app->m_moveDelta   = m_snapDeltas.x;
        g_app->m_rotateDelta = m_snapDeltas.y;
        g_app->m_scaleDelta  = m_snapDeltas.z;
      }
    }

    void EditorViewport::Init(Vec2 size)
    {
      m_needsResize = true;
      ComitResize();
      InitOverlays(this);
      m_snapDeltas = Vec3(0.25f, 45.0f, 0.25f);
    }

    void EditorViewport::LoadDragMesh(bool& meshLoaded,
                                      DirectoryEntry dragEntry,
                                      EntityPtr* dwMesh,
                                      LineBatchPtr* boundingBox,
                                      EditorScenePtr currScene)
    {
      if (!meshLoaded)
      {
        // Load mesh once
        String path = ConcatPaths({dragEntry.m_rootPath, dragEntry.m_fileName + dragEntry.m_ext});
        *dwMesh     = MakeNewPtr<Entity>();
        (*dwMesh)->AddComponent<MeshComponent>();

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

        if (mesh->IsSkinned())
        {
          SkeletonComponentPtr skelComp = (*dwMesh)->AddComponent<SkeletonComponent>();
          skelComp->SetSkeletonResourceVal(((SkinMesh*) mesh.get())->m_skeleton);

          skelComp->Init();
        }

        MaterialComponentPtr matComp = (*dwMesh)->AddComponent<MaterialComponent>();
        matComp->UpdateMaterialList();

        // Load bounding box once
        *boundingBox = CreateBoundingBoxDebugObject((*dwMesh)->GetBoundingBox(true));

        // Add bounding box to the scene
        currScene->AddEntity(*boundingBox);

        meshLoaded = true;
      }
    }

    Vec3 EditorViewport::CalculateDragMeshPosition(bool& meshLoaded,
                                                   EditorScenePtr currScene,
                                                   EntityPtr dwMesh,
                                                   LineBatchPtr* boundingBox)
    {
      Vec3 lastDragMeshPos = Vec3(0.0f);
      Ray ray              = RayFromMousePosition(); // Find the point of the cursor in 3D coordinates

      IDArray ignoreList;
      if (meshLoaded)
      {
        ignoreList.push_back((*boundingBox)->GetIdVal());
      }

      EditorScene::PickData pd = currScene->PickObject(ray, ignoreList);
      bool meshFound           = false;
      if (pd.entity != nullptr)
      {
        meshFound       = true;
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
        float firstY       = lastDragMeshPos.y;
        lastDragMeshPos.y -= dwMesh->GetBoundingBox(false).min.y;

        if (firstY > lastDragMeshPos.y)
        {
          lastDragMeshPos.y = firstY;
        }
      }

      (*boundingBox)->m_node->SetTranslation(lastDragMeshPos, TransformationSpace::TS_WORLD);

      return lastDragMeshPos;
    }

    void EditorViewport::HandleDropMesh(bool& meshLoaded,
                                        bool& meshAddedToScene,
                                        EditorScenePtr currScene,
                                        EntityPtr* dwMesh,
                                        LineBatchPtr* boundingBox)
    {
      if (meshLoaded && !ImGui::IsMouseDragging(0))
      {
        // Remove debug bounding box mesh from scene
        currScene->RemoveEntity((*boundingBox)->GetIdVal());
        meshLoaded = false;

        if (!meshAddedToScene)
        {
          *dwMesh = nullptr;
        }
        else
        {
          meshAddedToScene = false;
        }

        // Unload bounding box mesh
        *boundingBox = nullptr;
      }
    }

  } // namespace Editor
} // namespace ToolKit
