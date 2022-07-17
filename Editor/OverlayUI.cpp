#include "OverlayUI.h"
#include "EditorViewport.h"
#include "GlobalDef.h"
#include "Mod.h"
#include "ConsoleWindow.h"
#include "EditorCamera.h"
#include "EditorLight.h"
#include "EditorViewport2d.h"
#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    OverlayUI::OverlayUI(EditorViewport* owner)
    {
      m_owner = owner;
    }

    OverlayUI::~OverlayUI()
    {
    }

    void OverlayUI::SetOwnerState()
    {
      if (m_owner && m_owner->IsActive() && m_owner->IsVisible())
      {
        if (ImGui::IsWindowHovered())
        {
          m_owner->m_mouseOverOverlay = true;
        }
      }
    }

    // OverlayMods
    //////////////////////////////////////////////////////////////////////////

    OverlayMods::OverlayMods(EditorViewport* owner)
      : OverlayUI(owner)
    {
    }

    void OverlayMods::Show()
    {
      const float padding = 5.0f;
      Vec2 wndPos = Vec2
      (
        m_owner->m_wndPos.x + padding,
        m_owner->m_wndPos.y + padding
      ) + m_scroll;
      ImGui::SetNextWindowPos(wndPos);
      ImGui::SetNextWindowBgAlpha(0.65f);

      ImVec2 overlaySize(48, 260);
      if
      (
        ImGui::BeginChildFrame
        (
          ImGui::GetID("Navigation"),
          overlaySize,
          ImGuiWindowFlags_NoMove
          | ImGuiWindowFlags_NoDocking
          | ImGuiWindowFlags_NoTitleBar
          | ImGuiWindowFlags_NoResize
          | ImGuiWindowFlags_AlwaysAutoResize
          | ImGuiWindowFlags_NoSavedSettings
          | ImGuiWindowFlags_NoFocusOnAppearing
          | ImGuiWindowFlags_NoNav
          | ImGuiWindowFlags_NoScrollbar
          | ImGuiWindowFlags_NoScrollWithMouse
        )
      )
      {
        SetOwnerState();

        // Select button.
        bool isCurrentMod = ModManager::GetInstance()->m_modStack.back()->m_id
        == ModId::Select;
        ModManager::GetInstance()->SetMod
        (
          UI::ToggleButton
          (
            UI::m_selectIcn->m_textureId,
            ImVec2(32, 32),
            isCurrentMod
          ) && !isCurrentMod,
          ModId::Select
        );
        UI::HelpMarker
        (
          TKLoc + m_owner->m_name,
          "Select Box\nSelect items using box selection."
        );

        // Cursor button.
        isCurrentMod = ModManager::GetInstance()->m_modStack.back()->m_id
        == ModId::Cursor;
        ModManager::GetInstance()->SetMod
        (
          UI::ToggleButton
          (
            UI::m_cursorIcn->m_textureId,
            ImVec2(32, 32),
            isCurrentMod
          ) && !isCurrentMod,
          ModId::Cursor
        );
        UI::HelpMarker
        (
          TKLoc + m_owner->m_name,
          "Cursor\nSet the cursor location."
        );
        ImGui::Separator();

        // Move button.
        isCurrentMod = ModManager::GetInstance()->m_modStack.back()->m_id
        == ModId::Move;
        ModManager::GetInstance()->SetMod
        (
          UI::ToggleButton
          (
            UI::m_moveIcn->m_textureId,
            ImVec2(32, 32),
            isCurrentMod
          ) && !isCurrentMod,
          ModId::Move
        );
        UI::HelpMarker(TKLoc + m_owner->m_name, "Move\nMove selected items.");

        // Rotate button.
        isCurrentMod = ModManager::GetInstance()->m_modStack.back()->m_id
        == ModId::Rotate;
        ModManager::GetInstance()->SetMod
        (
          UI::ToggleButton
          (
            UI::m_rotateIcn->m_textureId,
            ImVec2(32, 32),
            isCurrentMod
          ) && !isCurrentMod,
          ModId::Rotate
        );
        UI::HelpMarker
        (
          TKLoc + m_owner->m_name,
          "Rotate\nRotate selected items."
        );

        // Scale button.
        isCurrentMod = ModManager::GetInstance()->m_modStack.back()->m_id
        == ModId::Scale;
        ModManager::GetInstance()->SetMod
        (
          UI::ToggleButton
          (
            UI::m_scaleIcn->m_textureId, ImVec2(32, 32),
            isCurrentMod
          ) && !isCurrentMod,
          ModId::Scale
        );
        UI::HelpMarker
        (
          TKLoc + m_owner->m_name,
          "Scale\nScale (resize) selected items."
        );
        ImGui::Separator();

        const char* items[] = { "1", "2", "4", "8", "16" };
        static int current_item = 3;  // Also the default.
        ImGui::PushItemWidth(40);
        if
        (
          ImGui::BeginCombo("##CS", items[current_item], ImGuiComboFlags_None)
        )
        {
          for (int n = 0; n < IM_ARRAYSIZE(items); n++)
          {
            bool is_selected = (current_item == n);
            if (ImGui::Selectable(items[n], is_selected))
            {
              current_item = n;
            }

            if (is_selected)
            {
              ImGui::SetItemDefaultFocus();
            }
          }
          ImGui::EndCombo();
        }
        ImGui::PopItemWidth();

        switch (current_item)
        {
        case 0:
          g_app->m_camSpeed = 0.5f;
          break;
        case 1:
          g_app->m_camSpeed = 1.0f;
          break;
        case 2:
          g_app->m_camSpeed = 2.0f;
          break;
        case 3:
          g_app->m_camSpeed = 4.0f;
          break;
        case 4:
          g_app->m_camSpeed = 16.0f;
          break;
        default:
          g_app->m_camSpeed = 8;
          break;
        }

        ImGuiStyle& style = ImGui::GetStyle();
        float spacing = style.ItemInnerSpacing.x;

        ImGui::SameLine(0, spacing);
        UI::HelpMarker(TKLoc + m_owner->m_name, "Camera speed m/s\n");
      }
      ImGui::EndChildFrame();
    }


    // OverlayViewportOptions Common Functions
    //////////////////////////////////////////////////////////////////////////

    void showAddMenu(void (*ShowAddMenuFn)(), uint32_t& nextItemIndex)
    {
      ImGui::TableSetColumnIndex(nextItemIndex++);
      ImGui::Image
      (
        Convert2ImGuiTexture(UI::m_worldIcon),
        ImVec2(20.0f, 20.0f)
      );

      ImGui::TableSetColumnIndex(nextItemIndex++);
      if (ImGui::Button("Add"))
      {
        ImGui::OpenPopup("##AddMenu");
      }

      if (ImGui::BeginPopup("##AddMenu"))
      {
        ShowAddMenuFn();
        ImGui::EndPopup();
      }
    }

    void showTransformOrientation
    (
      EditorViewport* m_owner,
      uint32_t& nextColumnItem
    )
    {
      bool change = false;
      ImGui::TableSetColumnIndex(nextColumnItem++);
      ImGui::Image
      (
        Convert2ImGuiTexture(UI::m_axisIcon),
        ImVec2(20.0f, 20.0f)
      );

      // Transform orientation combo.
      ImGuiStyle& style = ImGui::GetStyle();
      float spacing = style.ItemInnerSpacing.x;

      const char* itemsOrient[] = { "World", "Parent", "Local" };
      static int currentItemOrient = 0;

      ImGui::TableSetColumnIndex(nextColumnItem++);
      ImGui::PushItemWidth(72);
      if
        (
        ImGui::BeginCombo("##TRS",
        itemsOrient[currentItemOrient],
        ImGuiComboFlags_None)
        )
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
          ts = "parent";
          break;
          case 2:
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


    void autoSnapOptions(EditorViewport* m_owner, uint32_t& nextItemIndex)
    {
      // Auto snap.
      static bool autoSnapActivated = false;
      if (ImGui::GetIO().KeyCtrl)
      {
        if (!g_app->m_snapsEnabled)
        {
          autoSnapActivated = true;
          g_app->m_snapsEnabled = true;
        }
      }
      else if (autoSnapActivated)
      {
        autoSnapActivated = false;
        g_app->m_snapsEnabled = false;
      }

      ImGui::TableSetColumnIndex(nextItemIndex++);
      g_app->m_snapsEnabled = UI::ToggleButton
      (
        UI::m_snapIcon->m_textureId,
        ImVec2(16, 16),
        g_app->m_snapsEnabled
      );
      UI::HelpMarker
      (
        TKLoc + m_owner->m_name,
        "Grid snaping\nRight click for options"
      );


      if (ImGui::BeginPopupContextItem("##SnapMenu"))
      {
        ImGui::PushItemWidth(75);
        ImGui::InputFloat
        (
          "Move delta",
          &m_owner->m_snapDeltas.x,
          0.0f,
          0.0f,
          "%.2f"
        );
        ImGui::InputFloat
        (
          "Rotate delta",
          &m_owner->m_snapDeltas.y,
          0.0f,
          0.0f,
          "%.2f"
        );
        ImGui::InputFloat
        (
          "Scale delta",
          &m_owner->m_snapDeltas.z,
          0.0f,
          0.0f,
          "%.2f"
        );
        ImGui::PopItemWidth();
        ImGui::EndPopup();
      }
    }

    void cameraAlignmentOptions
    (
      EditorViewport* m_owner,
      uint32_t& nextItemIndex
    )
    {
      bool change = false;
      ImGui::TableSetColumnIndex(nextItemIndex++);
      ImGui::Image
      (
        Convert2ImGuiTexture(UI::m_cameraIcon),
        ImVec2(20.0f, 20.0f)
      );

      ImGui::TableSetColumnIndex(nextItemIndex++);
      m_owner->m_orbitLock = UI::ToggleButton
      (
        UI::m_lockIcon->m_textureId,
        ImVec2(16.0f, 16.0f),
        m_owner->m_orbitLock
      );
      UI::HelpMarker
      (
        TKLoc + m_owner->m_name,
        "Lock Camera Alignment\nMiddle button drag doesn't orbit."
        "\nOnly panning allowed."
      );

      // Camera alignment combo.
      const char* itemsCam[] = { "Free", "Top", "Front", "Left", "User" };
      int currentItemCam = static_cast<int>(m_owner->m_cameraAlignment);
      CameraAlignment rollBack = m_owner->m_cameraAlignment;

      ImGui::TableSetColumnIndex(nextItemIndex++);
      ImGui::PushItemWidth(72);
      if
        (
        ImGui::BeginCombo
        (
        "##VC",
        itemsCam[currentItemCam],
        ImGuiComboFlags_None
        )
        )
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
            currentItemCam = n;
            m_owner->m_cameraAlignment = (CameraAlignment)currentItemCam;
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
        switch ((CameraAlignment)currentItemCam)
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
            g_app->m_statusMsg = "Operation Failed !";
            g_app->GetConsole()->AddLog
            (
              "No camera selected.\nSelect a camera from the scene.",
              LogType::Error
            );
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
            String cmd = "SetCameraTransform --v \"" + m_owner->m_name
              + "\" " + view;
            g_app->GetConsole()->ExecCommand(cmd);
          }
        }
      }
      UI::HelpMarker(TKLoc + m_owner->m_name, "Camera Orientation\n");
    }

    // 2DView only
    void show2DViewZoomOptions(EditorViewport* m_owner, uint32_t& nextItemIndex)
    {
      EditorViewport2d* editorViewport =
        reinterpret_cast<EditorViewport2d*>(m_owner);
      ImGui::TableSetColumnIndex(nextItemIndex++);
      if (
        ImGui::ImageButton
        (
        Convert2ImGuiTexture(UI::m_viewZoomIcon),
        ImVec2(16, 16)
        ))
      {
        editorViewport->m_zoomPercentage = 100;
      }
      UI::HelpMarker(
        TKLoc + m_owner->m_name,
        "Reset Zoom");
      ImGui::TableSetColumnIndex(nextItemIndex++);
      ImGui::Text("%u%%", uint32_t(editorViewport->m_zoomPercentage));
    }

    // 2DView only
    void showGridOptions(EditorViewport* m_owner, uint32_t& nextItemIndex)
    {
      auto ShowGridOptionsFn = [m_owner]() -> void
      {
        ImGui::PushItemWidth(75);
        EditorViewport2d* editorViewport =
          reinterpret_cast<EditorViewport2d*>(m_owner);
        static constexpr uint16_t cellSizeStep = 5, gridSizeStep = 0;
        ImGui::InputScalar
        (
          "Cell Size",
          ImGuiDataType_U16, &editorViewport->m_gridCellSizeByPixel,
          &cellSizeStep
        );
        ImGui::InputInt2("Grid Size",
          reinterpret_cast<int*>(&editorViewport->m_gridWholeSize));
        ImGui::PopItemWidth();
      };


      ImGui::TableSetColumnIndex(nextItemIndex++);
      if (
        ImGui::ImageButton
        (
        (ImTextureID)UI::m_gridIcon->m_textureId, ImVec2(18, 18)
        )
        )
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



    // OverlayViewportOptions
    //////////////////////////////////////////////////////////////////////////

    OverlayViewportOptions::OverlayViewportOptions(EditorViewport* owner)
      : OverlayUI(owner)
    {
    }

    void OverlayViewportOptions::Show()
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
            Cone* cone = new Cone({ 1.0f, 1.0f, 30, 30 });
            cone->GetMeshComponent()->Init(false);
            currScene->AddEntity(cone);
          }
          if (ImGui::MenuItem("Monkey"))
          {
            Drawable* suzanne = new Drawable();
            suzanne->SetMesh
            (
              GetMeshManager()->Create<Mesh>(MeshPath("suzanne.mesh", true))
            );

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
            Surface* suface = new Surface
            (
              Vec2(100.0f, 30.0f),
              Vec2 (0.0f, 0.0f)
            );
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
          Entity* node = Entity::CreateByType(EntityType::Entity_Node);
          currScene->AddEntity(node);
        }

        if (ImGui::MenuItem("Camera"))
        {
          Entity* node = new EditorCamera();
          currScene->AddEntity(node);
        }

        if (ImGui::BeginMenu("Light"))
        {
          if (ImGui::MenuItem("Sun"))
          {
            EditorDirectionalLight* light = new EditorDirectionalLight();
            light->Init();
            light->SetNameVal("Sun");
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
          ImGui::EndMenu();
        }
      };

      ImVec2 overlaySize(360, 30);

      // Center the toolbar.
      float width = ImGui::GetWindowContentRegionWidth();
      float offset = (width - overlaySize.x) * 0.5f;
      ImGui::SameLine(offset);

      ImGui::SetNextWindowBgAlpha(0.65f);
      if
      (
        ImGui::BeginChildFrame
        (
          ImGui::GetID("ViewportOptions"),
          overlaySize,
          ImGuiWindowFlags_NoMove |
          ImGuiWindowFlags_NoTitleBar |
          ImGuiWindowFlags_NoScrollbar |
          ImGuiWindowFlags_NoScrollWithMouse
        )
      )
      {
        SetOwnerState();

        ImGui::BeginTable
        (
          "##SettingsBar",
          8,
          ImGuiTableFlags_SizingStretchProp
        );
        ImGui::TableNextRow();
        unsigned int nextItemIndex = 0;

        showAddMenu(ShowAddMenuFn, nextItemIndex);

        cameraAlignmentOptions(m_owner, nextItemIndex);

        showTransformOrientation(m_owner, nextItemIndex);

        autoSnapOptions(m_owner, nextItemIndex);

        ImGui::EndTable();
      }
      ImGui::EndChildFrame();
    }

    // Overlay2DViewportOptions
    //////////////////////////////////////////////////////////////////////////

    Overlay2DViewportOptions::Overlay2DViewportOptions(EditorViewport * owner)
        : OverlayUI(owner)
    {
    }

    void Overlay2DViewportOptions::Show()
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
          Surface* suface = new Surface
          (
            Vec2(100.0f, 30.0f),
            Vec2(0.0f, 0.0f)
          );
          suface->GetMeshComponent()->Init(false);
          currScene->AddEntity(suface);
        }

        if (ImGui::MenuItem("Button"))
        {
          Surface* suface = new Button(Vec2(100.0f, 30.0f));
          suface->GetMeshComponent()->Init(false);
          currScene->AddEntity(suface);
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Node"))
        {
          Entity* node = Entity::CreateByType(EntityType::Entity_Node);
          currScene->AddEntity(node);
        }
      };

      ImVec2 overlaySize(300, 34);

      // Center the toolbar.
      float width = ImGui::GetWindowContentRegionWidth();
      float offset = (width - overlaySize.x) * 0.5f;
      ImGui::SameLine(offset);

      ImGui::SetNextWindowBgAlpha(0.65f);
      if
      (
        ImGui::BeginChildFrame
        (
          ImGui::GetID("ViewportOptions"),
          overlaySize,
          ImGuiWindowFlags_NoMove
          | ImGuiWindowFlags_NoTitleBar
          | ImGuiWindowFlags_NoScrollbar
          | ImGuiWindowFlags_NoScrollWithMouse
        )
      )
      {
        SetOwnerState();

        ImGui::BeginTable
        (
          "##SettingsBar",
          8,
          ImGuiTableFlags_SizingStretchProp
        );
        ImGui::TableNextRow();

        unsigned int nextItemIndex = 0;

        showAddMenu(ShowAddMenuFn, nextItemIndex);

        showTransformOrientation(m_owner, nextItemIndex);

        autoSnapOptions(m_owner, nextItemIndex);

        show2DViewZoomOptions(m_owner, nextItemIndex);

        showGridOptions(m_owner, nextItemIndex);

        ImGui::EndTable();
      }
      ImGui::EndChildFrame();
    }

    // StatusBar
    //////////////////////////////////////////////////////////////////////////

    StatusBar::StatusBar(EditorViewport* owner)
      : OverlayUI(owner)
    {
    }

    void StatusBar::Show()
    {
      // Status bar.
      ImVec2 overlaySize;
      overlaySize.x = m_owner->m_width - 2;
      overlaySize.y = 24;
      Vec2 pos = m_owner->m_wndPos;
      Vec2 wndPadding = ImGui::GetStyle().WindowPadding;

      pos.x += 1;
      pos.y += m_owner->m_height - wndPadding.y - 16.0f;
      ImGui::SetNextWindowPos(pos + m_scroll);
      ImGui::SetNextWindowBgAlpha(0.65f);
      if
      (
        ImGui::BeginChildFrame
        (
          ImGui::GetID("ProjectInfo"),
          overlaySize,
          ImGuiWindowFlags_NoMove
          | ImGuiWindowFlags_NoTitleBar
          | ImGuiWindowFlags_NoScrollbar
          | ImGuiWindowFlags_NoScrollWithMouse
        )
      )
      {
        String info = "Status: ";
        ImGui::Text(info.c_str());

        // If the status message has changed.
        static String prevMsg = g_app->m_statusMsg;
        if (g_app->m_statusMsg != "OK")
        {
          // Hold msg for 3 sec. before switching to OK.
          static float elapsedTime = 0.0f;
          elapsedTime += ImGui::GetIO().DeltaTime;

          // For overlapping message updates,
          // always reset timer for the last event.
          if (prevMsg != g_app->m_statusMsg)
          {
            elapsedTime = 0.0f;
            prevMsg = g_app->m_statusMsg;
          }

          if (elapsedTime > 3)
          {
            elapsedTime = 0.0f;
            g_app->m_statusMsg = "OK";
          }
        }

        // Inject status.
        ImGui::SameLine();
        ImGui::Text(g_app->m_statusMsg.c_str());

        ImVec2 msgSize = ImGui::CalcTextSize(g_app->m_statusMsg.c_str());
        float wndWidth = ImGui::GetWindowContentRegionWidth();

        // If there is enough space for info.
        if (wndWidth * 0.3f > msgSize.x)
        {
          // Draw Projcet Info.
          Project prj = g_app->m_workspace.GetActiveProject();
          info = "Project: " + prj.name + "Scene: " + prj.scene;
          pos = ImGui::CalcTextSize(info.c_str());

          ImGui::SameLine((m_owner->m_width - pos.x) * 0.5f);
          info = "Project: " + prj.name;
          ImGui::BulletText(info.c_str());
          ImGui::SameLine();
          info = "Scene: " + prj.scene;
          ImGui::BulletText(info.c_str());

          // Draw Fps.
          String fps = "Fps: " + std::to_string(g_app->m_fps);
          ImGui::SameLine(m_owner->m_width - 70.0f);
          ImGui::Text(fps.c_str());
        }
      }
      ImGui::EndChildFrame();
    }

  }  // namespace Editor
}  // namespace ToolKit
