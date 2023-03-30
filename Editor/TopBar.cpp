#include "TopBar.h"

#include "App.h"
#include "EditorCamera.h"
#include "GradientSky.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    OverlayTopBar::OverlayTopBar(EditorViewport* owner) : OverlayUI(owner) {}

    void OverlayTopBar::Show()
    {
      assert(m_owner);
      if (m_owner == nullptr)
      {
        return;
      }

      auto ShowAddMenuFn = []() -> void
      {
        EditorScenePtr currScene = g_app->GetCurrentScene();
        if (ImGui::BeginMenu("Mesh"))
        {
          if (ImGui::MenuItem("Plane"))
          {
            Quad* plane = new Quad();
            plane->GetMeshComponent()->Init(false);
            currScene->AddEntity(plane);
          }
          if (ImGui::MenuItem("Cube"))
          {
            Cube* cube = new Cube();
            cube->GetMeshComponent()->Init(false);
            currScene->AddEntity(cube);
          }
          if (ImGui::MenuItem("Sphere"))
          {
            Sphere* sphere = new Sphere();
            sphere->GetMeshComponent()->Init(false);
            currScene->AddEntity(sphere);
          }
          if (ImGui::MenuItem("Cone"))
          {
            Cone* cone = new Cone({1.0f, 1.0f, 30, 30});
            cone->GetMeshComponent()->Init(false);
            currScene->AddEntity(cone);
          }
          if (ImGui::MenuItem("Monkey"))
          {
            Drawable* suzanne = new Drawable();
            suzanne->SetMesh(
                GetMeshManager()->Create<Mesh>(MeshPath("suzanne.mesh", true)));

            suzanne->GetMesh()->Init(false);
            currScene->AddEntity(suzanne);
          }
          ImGui::EndMenu();
        }
        ImGui::Separator();

        if (ImGui::BeginMenu("2D UI"))
        {
          if (ImGui::MenuItem("Surface"))
          {
            Surface* suface =
                new Surface(Vec2(100.0f, 30.0f), Vec2(0.0f, 0.0f));
            suface->GetMeshComponent()->Init(false);
            currScene->AddEntity(suface);
          }

          if (ImGui::MenuItem("Button"))
          {
            Surface* suface = new Button(Vec2(100.0f, 30.0f));
            suface->GetMeshComponent()->Init(false);
            currScene->AddEntity(suface);
          }
          ImGui::EndMenu();
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Node"))
        {
          Entity* node =
              GetEntityFactory()->CreateByType(EntityType::Entity_Node);
          currScene->AddEntity(node);
        }

        if (ImGui::MenuItem("Camera"))
        {
          Entity* node = new EditorCamera();
          currScene->AddEntity(node);
        }

        if (ImGui::BeginMenu("Light"))
        {
          if (ImGui::MenuItem("Directional"))
          {
            EditorDirectionalLight* light = new EditorDirectionalLight();
            light->Init();
            light->SetNameVal("DirectionalLight");
            currScene->AddEntity(static_cast<Entity*>(light));
          }

          if (ImGui::MenuItem("Point"))
          {
            EditorPointLight* light = new EditorPointLight();
            light->Init();
            light->SetNameVal("PointLight");
            currScene->AddEntity(static_cast<Entity*>(light));
          }

          if (ImGui::MenuItem("Spot"))
          {
            EditorSpotLight* light = new EditorSpotLight();
            light->Init();
            light->SetNameVal("SpotLight");
            currScene->AddEntity(static_cast<Entity*>(light));
          }

          if (ImGui::MenuItem("Sky"))
          {
            currScene->AddEntity(new Sky());
          }

          if (ImGui::MenuItem("Gradient Sky"))
          {
            currScene->AddEntity(new GradientSky());
          }

          ImGui::EndMenu();
        }
      };

      ImVec2 overlaySize(360, 30);

      // Center the toolbar.
      float width  = ImGui::GetWindowContentRegionWidth();
      float offset = (width - overlaySize.x) * 0.5f;
      ImGui::SameLine(offset);

      ImGui::SetNextWindowBgAlpha(0.65f);
      if (ImGui::BeginChildFrame(ImGui::GetID("ViewportOptions"),
                                 overlaySize,
                                 ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoTitleBar |
                                     ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoScrollWithMouse))
      {
        SetOwnerState();

        ImGui::BeginTable("##SettingsBar",
                          8,
                          ImGuiTableFlags_SizingStretchProp);
        ImGui::TableNextRow();
        unsigned int nextItemIndex = 0;

        ShowAddMenu(ShowAddMenuFn, nextItemIndex);

        CameraAlignmentOptions(nextItemIndex);

        ShowTransformOrientation(nextItemIndex);

        SnapOptions(nextItemIndex);

        ImGui::EndTable();
      }
      ImGui::EndChildFrame();
    }

    void OverlayTopBar::ShowAddMenu(std::function<void()> showMenuFn,
                                    uint32_t& nextItemIndex)
    {
      ImGui::TableSetColumnIndex(nextItemIndex++);
      ImGui::Image(Convert2ImGuiTexture(UI::m_worldIcon), ImVec2(20.0f, 20.0f));

      ImGui::TableSetColumnIndex(nextItemIndex++);
      if (ImGui::Button("Add"))
      {
        ImGui::OpenPopup("##AddMenu");
      }

      if (ImGui::BeginPopup("##AddMenu"))
      {
        showMenuFn();
        ImGui::EndPopup();
      }
    }

    void OverlayTopBar::ShowTransformOrientation(uint32_t& nextColumnItem)
    {
      bool change = false;
      ImGui::TableSetColumnIndex(nextColumnItem++);
      ImGui::Image(Convert2ImGuiTexture(UI::m_axisIcon), ImVec2(20.0f, 20.0f));

      // Transform orientation combo.
      ImGuiStyle& style            = ImGui::GetStyle();
      float spacing                = style.ItemInnerSpacing.x;

      const char* itemsOrient[]    = {"World", "Local"};
      static int currentItemOrient = 0;

      ImGui::TableSetColumnIndex(nextColumnItem++);
      ImGui::PushItemWidth(72);
      if (ImGui::BeginCombo("##TRS",
                            itemsOrient[currentItemOrient],
                            ImGuiComboFlags_None))
      {
        for (int n = 0; n < IM_ARRAYSIZE(itemsOrient); n++)
        {
          bool is_selected = (currentItemOrient == n);
          if (ImGui::Selectable(itemsOrient[n], is_selected))
          {
            if (currentItemOrient != n)
            {
              change = true;
            }
            currentItemOrient = n;
          }

          if (is_selected)
          {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      if (change)
      {
        String ts;
        switch (currentItemOrient)
        {
        case 1:
          ts = "local";
          break;
        case 0:
        default:
          ts = "world";
          break;
        }

        String cmd = "SetTransformOrientation " + ts;
        g_app->GetConsole()->ExecCommand(cmd);
      }
      UI::HelpMarker(TKLoc + m_owner->m_name, "Transform orientations\n");
    }

    void OverlayTopBar::SnapOptions(uint32_t& nextItemIndex)
    {
      // Auto snap.
      static bool autoSnapActivated = false;
      if (ImGui::GetIO().KeyCtrl)
      {
        if (!g_app->m_snapsEnabled)
        {
          autoSnapActivated     = true;
          g_app->m_snapsEnabled = true;
        }
      }
      else if (autoSnapActivated)
      {
        autoSnapActivated     = false;
        g_app->m_snapsEnabled = false;
      }

      ImGui::TableSetColumnIndex(nextItemIndex++);
      g_app->m_snapsEnabled = UI::ToggleButton(UI::m_snapIcon->m_textureId,
                                               ImVec2(16, 16),
                                               g_app->m_snapsEnabled);
      UI::HelpMarker(TKLoc + m_owner->m_name,
                     "Grid snaping\nRight click for options");

      if (ImGui::BeginPopupContextItem("##SnapMenu"))
      {
        ImGui::PushItemWidth(75);
        ImGui::InputFloat("Move delta",
                          &m_owner->m_snapDeltas.x,
                          0.0f,
                          0.0f,
                          "%.2f");
        ImGui::InputFloat("Rotate delta",
                          &m_owner->m_snapDeltas.y,
                          0.0f,
                          0.0f,
                          "%.2f");
        ImGui::InputFloat("Scale delta",
                          &m_owner->m_snapDeltas.z,
                          0.0f,
                          0.0f,
                          "%.2f");
        ImGui::PopItemWidth();
        ImGui::EndPopup();
      }
    }

    void OverlayTopBar::CameraAlignmentOptions(uint32_t& nextItemIndex)
    {
      bool change = false;
      ImGui::TableSetColumnIndex(nextItemIndex++);
      ImGui::Image(Convert2ImGuiTexture(UI::m_cameraIcon),
                   ImVec2(20.0f, 20.0f));

      ImGui::TableSetColumnIndex(nextItemIndex++);
      m_owner->m_orbitLock = UI::ToggleButton(UI::m_lockIcon->m_textureId,
                                              ImVec2(16.0f, 16.0f),
                                              m_owner->m_orbitLock);
      UI::HelpMarker(TKLoc + m_owner->m_name,
                     "Lock Camera Alignment\nMiddle button drag doesn't orbit."
                     "\nOnly panning allowed.");

      // Camera alignment combo.
      const char* itemsCam[]   = {"Free", "Top", "Front", "Left", "User"};
      int currentItemCam       = static_cast<int>(m_owner->m_cameraAlignment);
      CameraAlignment rollBack = m_owner->m_cameraAlignment;

      ImGui::TableSetColumnIndex(nextItemIndex++);
      ImGui::PushItemWidth(72);
      if (ImGui::BeginCombo("##VC",
                            itemsCam[currentItemCam],
                            ImGuiComboFlags_None))
      {
        for (int n = 0; n < IM_ARRAYSIZE(itemsCam); n++)
        {
          bool isSelected = (currentItemCam == n);
          if (ImGui::Selectable(itemsCam[n], isSelected))
          {
            if (currentItemCam != n)
            {
              change = true;
            }
            currentItemCam             = n;
            m_owner->m_cameraAlignment = (CameraAlignment) currentItemCam;
          }

          if (isSelected)
          {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      if (change)
      {
        String view;
        switch ((CameraAlignment) currentItemCam)
        {
        case CameraAlignment::Top:
          view = "Top";
          break;
        case CameraAlignment::Front:
          view = "Front";
          break;
        case CameraAlignment::Left:
          view = "Left";
          break;
        case CameraAlignment::User:
          view = "User";
          break;
        case CameraAlignment::Free:
        default:
          view = "Free";
          break;
        }

        if (view == "User")
        {
          bool noCamera = true;
          if (Entity* cam = g_app->GetCurrentScene()->GetCurrentSelection())
          {
            if (cam->GetType() == EntityType::Entity_Camera)
            {
              if (EditorViewport* vp = g_app->GetActiveViewport())
              {
                vp->AttachCamera(cam->GetIdVal());
                noCamera = false;
              }
            }
          }

          if (noCamera)
          {
            m_owner->m_cameraAlignment = rollBack;
            g_app->m_statusMsg         = "Operation Failed !";
            g_app->GetConsole()->AddLog(
                "No camera selected.\nSelect a camera from the scene.",
                LogType::Error);
          }
        }
        else
        {
          if (EditorViewport* vp = g_app->GetActiveViewport())
          {
            vp->AttachCamera(NULL_HANDLE);
          }

          if (view != "Free")
          {
            m_owner->m_cameraAlignment = CameraAlignment::Free;
            String cmd =
                "SetCameraTransform --v \"" + m_owner->m_name + "\" " + view;
            g_app->GetConsole()->ExecCommand(cmd);
          }
        }
      }
      UI::HelpMarker(TKLoc + m_owner->m_name, "Camera Orientation\n");
    }

  } // namespace Editor
} // namespace ToolKit