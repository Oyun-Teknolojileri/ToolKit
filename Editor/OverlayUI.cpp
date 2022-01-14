#include "stdafx.h"

#include "OverlayUI.h"
#include "EditorViewport.h"
#include "GlobalDef.h"
#include "Mod.h"
#include "ConsoleWindow.h"
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
      Vec2 wndPos = Vec2(m_owner->m_wndPos.x + padding, m_owner->m_wndPos.y + padding) + m_scroll;
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
        bool isCurrentMod = ModManager::GetInstance()->m_modStack.back()->m_id == ModId::Select;
        ModManager::GetInstance()->SetMod
        (
          UI::ToggleButton((void*)(intptr_t)UI::m_selectIcn->m_textureId, ImVec2(32, 32), isCurrentMod) && !isCurrentMod,
          ModId::Select
        );
        UI::HelpMarker(LOC + m_owner->m_name, "Select Box\nSelect items using box selection.");

        // Cursor button.
        isCurrentMod = ModManager::GetInstance()->m_modStack.back()->m_id == ModId::Cursor;
        ModManager::GetInstance()->SetMod
        (
          UI::ToggleButton((void*)(intptr_t)UI::m_cursorIcn->m_textureId, ImVec2(32, 32), isCurrentMod) && !isCurrentMod,
          ModId::Cursor
        );
        UI::HelpMarker(LOC + m_owner->m_name, "Cursor\nSet the cursor location.");
        ImGui::Separator();

        // Move button.
        isCurrentMod = ModManager::GetInstance()->m_modStack.back()->m_id == ModId::Move;
        ModManager::GetInstance()->SetMod
        (
          UI::ToggleButton((void*)(intptr_t)UI::m_moveIcn->m_textureId, ImVec2(32, 32), isCurrentMod) && !isCurrentMod,
          ModId::Move
        );
        UI::HelpMarker(LOC + m_owner->m_name, "Move\nMove selected items.");

        // Rotate button.
        isCurrentMod = ModManager::GetInstance()->m_modStack.back()->m_id == ModId::Rotate;
        ModManager::GetInstance()->SetMod
        (
          UI::ToggleButton((void*)(intptr_t)UI::m_rotateIcn->m_textureId, ImVec2(32, 32), isCurrentMod) && !isCurrentMod,
          ModId::Rotate
        );
        UI::HelpMarker(LOC + m_owner->m_name, "Rotate\nRotate selected items.");

        // Scale button.
        isCurrentMod = ModManager::GetInstance()->m_modStack.back()->m_id == ModId::Scale;
        ModManager::GetInstance()->SetMod
        (
          UI::ToggleButton((void*)(intptr_t)UI::m_scaleIcn->m_textureId, ImVec2(32, 32), isCurrentMod) && !isCurrentMod,
          ModId::Scale
        );
        UI::HelpMarker(LOC + m_owner->m_name, "Scale\nScale (resize) selected items.");
        ImGui::Separator();

        const char* items[] = { "1", "2", "4", "8", "16" };
        static int current_item = 3; // Also the default.
        ImGui::PushItemWidth(40);
        if (ImGui::BeginCombo("##CS", items[current_item], ImGuiComboFlags_None))
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
        UI::HelpMarker(LOC + m_owner->m_name, "Camera speed m/s\n");
      }
      ImGui::EndChildFrame();
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
        if (ImGui::BeginMenu("Mesh"))
        {
          if (ImGui::MenuItem("Plane"))
          {
            Quad* plane = new Quad();
            plane->m_mesh->Init(false);
            g_app->m_scene->AddEntity(plane);
          }
          if (ImGui::MenuItem("Cube"))
          {
            Cube* cube = new Cube();
            cube->m_mesh->Init(false);
            g_app->m_scene->AddEntity(cube);
          }
          if (ImGui::MenuItem("Sphere"))
          {
            Sphere* sphere = new Sphere();
            sphere->m_mesh->Init(false);
            g_app->m_scene->AddEntity(sphere);
          }
          if (ImGui::MenuItem("Cone"))
          {
            Cone* cone = new Cone({ 1.0f, 1.0f, 30, 30 });
            cone->m_mesh->Init(false);
            g_app->m_scene->AddEntity(cone);
          }
          if (ImGui::MenuItem("Monkey"))
          {
            Drawable* suzanne = new Drawable();
            suzanne->m_mesh = GetMeshManager()->Create<Mesh>(MeshPath("suzanne.mesh", true));
            suzanne->m_mesh->Init(false);
            g_app->m_scene->AddEntity(suzanne);
          }
          ImGui::EndMenu();
        }
        ImGui::Separator();

        if (ImGui::BeginMenu("2D UI"))
        {
          if (ImGui::MenuItem("Surface"))
          {
            Surface* suface = new Surface(Vec2(100.0f, 30.0f), Vec2 (0.0f, 0.0f));
            suface->m_mesh->Init(false);
            g_app->m_scene->AddEntity(suface);
          }

          if (ImGui::MenuItem("Button"))
          {
            Surface* suface = new Button(Vec2(100.0f, 30.0f));
            suface->m_mesh->Init(false);
            g_app->m_scene->AddEntity(suface);
          }
          ImGui::EndMenu();
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Node"))
        {
          Entity* node = Entity::CreateByType(EntityType::Entity_Node);
          g_app->m_scene->AddEntity(node);
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
          ImGuiWindowFlags_NoMove
          | ImGuiWindowFlags_NoTitleBar
          | ImGuiWindowFlags_NoScrollbar
          | ImGuiWindowFlags_NoScrollWithMouse
        )
      )
      {
        SetOwnerState();

        ImGui::BeginTable("##SettingsBar", 8, ImGuiTableFlags_SizingStretchProp);
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::Image(Convert2ImGuiTexture(UI::m_worldIcon), ImVec2(20.0f, 20.0f));

        ImGui::TableSetColumnIndex(1);
        if (ImGui::Button("Add"))
        {
          ImGui::OpenPopup("##AddMenu");
        }

        if (ImGui::BeginPopup("##AddMenu"))
        {
          ShowAddMenuFn();
          ImGui::EndPopup();
        }

        ImGui::TableSetColumnIndex(2);
        ImGui::Image(Convert2ImGuiTexture(UI::m_cameraIcon), ImVec2(20.0f, 20.0f));

        ImGui::TableSetColumnIndex(3);
        m_owner->m_orbitLock = UI::ToggleButton(Convert2ImGuiTexture(UI::m_lockIcon), ImVec2(16.0f, 16.0f), m_owner->m_orbitLock);
        UI::HelpMarker(LOC + m_owner->m_name, "Lock Camera Alignment\nMiddle button drag doesn't orbit.\nOnly panning allowed.");

        // Camera alignment combo.
        const char* itemsCam[] = { "Free", "Top", "Front", "Left" };
        int currentItemCam = m_owner->m_cameraAlignment;
        bool change = false;

        ImGui::TableSetColumnIndex(4);
        ImGui::PushItemWidth(72);
        if (ImGui::BeginCombo("##VC", itemsCam[currentItemCam], ImGuiComboFlags_None))
        {
          for (int n = 0; n < IM_ARRAYSIZE(itemsCam); n++)
          {
            bool is_selected = (currentItemCam == n);
            if (ImGui::Selectable(itemsCam[n], is_selected))
            {
              if (currentItemCam != n)
              {
                change = true;
              }
              currentItemCam = n;
              m_owner->m_cameraAlignment = currentItemCam;
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
          String view;
          switch (currentItemCam)
          {
          case 1:
            view = "top";
            break;
          case 2:
            view = "front";
            break;
          case 3:
            view = "left";
            break;
          case 0:
          default:
            view = "free";
            break;
          }

          if (view != "free")
          {
            String cmd = "SetCameraTransform --v \"" + m_owner->m_name + "\" " + view;
            g_app->GetConsole()->ExecCommand(cmd);
          }
        }
        UI::HelpMarker(LOC + m_owner->m_name, "Camera Orientation\n");

        ImGui::TableSetColumnIndex(5);
        ImGui::Image(Convert2ImGuiTexture(UI::m_axisIcon), ImVec2(20.0f, 20.0f));

        // Transform orientation combo.
        ImGuiStyle& style = ImGui::GetStyle();
        float spacing = style.ItemInnerSpacing.x;

        const char* itemsOrient[] = { "World", "Parent", "Local" };
        static int currentItemOrient = 0;

        change = false;
        ImGui::TableSetColumnIndex(6);
        ImGui::PushItemWidth(72);
        if (ImGui::BeginCombo("##TRS", itemsOrient[currentItemOrient], ImGuiComboFlags_None))
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
        UI::HelpMarker(LOC + m_owner->m_name, "Transform orientations\n");

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

        ImGui::TableSetColumnIndex(7);
        g_app->m_snapsEnabled = UI::ToggleButton((void*)(intptr_t)UI::m_snapIcon->m_textureId, ImVec2(16, 16), g_app->m_snapsEnabled);
        UI::HelpMarker(LOC + m_owner->m_name, "Grid snaping\nRight click for options");

        if (ImGui::BeginPopupContextItem("##SnapMenu"))
        {
          ImGui::PushItemWidth(75);
          ImGui::InputFloat("Move delta", &g_app->m_moveDelta, 0.0f, 0.0f, "%.2f");
          ImGui::InputFloat("Rotate delta", &g_app->m_rotateDelta, 0.0f, 0.0f, "%.2f");
          ImGui::InputFloat("Scale delta", &g_app->m_scaleDelta, 0.0f, 0.0f, "%.2f");
          ImGui::PopItemWidth();
          ImGui::EndPopup();
        }

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
      ImGui::EndChildFrame();
    }

}
}
