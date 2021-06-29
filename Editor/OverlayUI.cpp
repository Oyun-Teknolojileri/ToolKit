#include "stdafx.h"

#include "OverlayUI.h"
#include "Viewport.h"
#include "GlobalDef.h"
#include "Mod.h"
#include "ConsoleWindow.h"
#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    OverlayUI::OverlayUI(Viewport* owner)
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

    OverlayMods::OverlayMods(Viewport* owner)
      : OverlayUI(owner)
    {
    }

    void OverlayMods::Show()
    {
      ImVec2 overlaySize(48, 260);
      const float padding = 5.0f;
      ImVec2 window_pos = ImVec2(m_owner->m_wndPos.x + padding, m_owner->m_wndPos.y + padding);
      ImGui::SetNextWindowPos(window_pos);
      ImGui::SetNextWindowBgAlpha(0.65f);
      if (ImGui::BeginChildFrame(ImGui::GetID("Navigation"), overlaySize, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
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

        ImGui::EndChildFrame();
      }
    }

    // OverlayViewportOptions
    //////////////////////////////////////////////////////////////////////////

    OverlayViewportOptions::OverlayViewportOptions(Viewport* owner)
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

      auto ShowAddMenuFn = []()
      {
        if (ImGui::BeginMenu("Mesh"))
        {
          if (ImGui::MenuItem("Plane"))
          {
            Quad* plane = new Quad();
            plane->m_mesh->Init(false);
            g_app->m_scene.AddEntity(plane);
          }
          if (ImGui::MenuItem("Cube"))
          {
            Cube* cube = new Cube();
            cube->m_mesh->Init(false);
            g_app->m_scene.AddEntity(cube);
          }
          if (ImGui::MenuItem("UV Sphere"))
          {
            Sphere* sphere = new Sphere();
            sphere->m_mesh->Init(false);
            g_app->m_scene.AddEntity(sphere);
          }
          if (ImGui::MenuItem("Cylinder"))
          {
          }
          if (ImGui::MenuItem("Cone"))
          {
            Cone* cone = new Cone({ 1.0f, 1.0f, 30, 30 });
            cone->m_mesh->Init(false);
            g_app->m_scene.AddEntity(cone);
          }
          if (ImGui::MenuItem("Monkey"))
          {
            Drawable* suzanne = new Drawable();
            suzanne->m_mesh = GetMeshManager()->Create(MeshPath("suzanne.mesh"));
            suzanne->m_mesh->Init(false);
            g_app->m_scene.AddEntity(suzanne);
          }
          ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Light"))
        {
          if (ImGui::MenuItem("Point"))
          {
          }
          if (ImGui::MenuItem("Sun"))
          {
          }
          if (ImGui::MenuItem("Spot"))
          {
          }
          if (ImGui::MenuItem("Area"))
          {
          }
          ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Camera"))
        {
        }
        if (ImGui::MenuItem("Speaker"))
        {
        }
        if (ImGui::BeginMenu("Light Probe"))
        {
          if (ImGui::MenuItem("Reflection Cubemap"))
          {
          }
          if (ImGui::MenuItem("Reflection Plane"))
          {
          }
          if (ImGui::MenuItem("Irradiance Volume"))
          {
          }
          ImGui::EndMenu();
        }
      };

      ImVec2 overlaySize(320, 30);

      const float padding = 5.0f;
      ImVec2 window_pos = ImVec2(m_owner->m_wndPos.x + padding + 52, m_owner->m_wndPos.y + padding);
      ImGui::SetNextWindowPos(window_pos);
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

        ImGui::Image(Convert2ImGuiTexture(UI::m_worldIcon), ImVec2(20.0f, 20.0f));
        ImGui::SameLine();

        if (ImGui::Button("Add"))
        {
          ImGui::OpenPopup("##AddMenu");
        }
        ImGui::SameLine();

        if (ImGui::BeginPopup("##AddMenu"))
        {
          ShowAddMenuFn();
          ImGui::EndPopup();
        }

        ImGui::Image(Convert2ImGuiTexture(UI::m_cameraIcon), ImVec2(20.0f, 20.0f));
        ImGui::SameLine();

        // Camera alignment combo.
        const char* itemsCam[] = { "Free", "Top", "Front", "Left" };
        int currentItemCam = m_owner->m_cameraAlignment;
        bool change = false;

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
        ImGui::SameLine();
        UI::HelpMarker(LOC + m_owner->m_name, "Camera Orientation\n");

        ImGui::Image(Convert2ImGuiTexture(UI::m_axisIcon), ImVec2(20.0f, 20.0f));
        ImGui::SameLine();

        // Transform orientation combo.
        ImGuiStyle& style = ImGui::GetStyle();
        float spacing = style.ItemInnerSpacing.x;

        const char* itemsOrient[] = { "World", "Parent", "Local" };
        static int currentItemOrient = 0;

        change = false;
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

        ImGui::SameLine(0, spacing); 
        UI::HelpMarker(LOC + m_owner->m_name, "Transform orientations\n");

        // Snap Bar.
        ImGui::Separator();

        // Snap button.
        ImGui::SameLine(0, spacing);
        static float hoverTimeSnapBtn = 0.0f;

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

        g_app->m_snapsEnabled = UI::ToggleButton((void*)(intptr_t)UI::m_snapIcon->m_textureId, ImVec2(16, 16), g_app->m_snapsEnabled);

        if (ImGui::BeginPopupContextItem("##SnapMenu"))
        {
          ImGui::PushItemWidth(75);
          ImGui::InputFloat("Move delta", &g_app->m_moveDelta, 0.0f, 0.0f, "%.2f");
          ImGui::InputFloat("Rotate delta", &g_app->m_rotateDelta, 0.0f, 0.0f, "%.2f");
          ImGui::InputFloat("Scale delta", &g_app->m_scaleDelta, 0.0f, 0.0f, "%.2f");
          ImGui::PopItemWidth();
          ImGui::Checkbox("Snap to grid", &g_app->m_snapToGrid);

          ImGui::EndPopup();
        }

        ImGui::EndChildFrame();
      }

    }

  }
}
