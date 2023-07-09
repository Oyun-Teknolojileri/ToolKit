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

#include "TopBar.h"

#include "App.h"
#include "EditorCamera.h"
#include "IconsFontAwesome.h"
#include "OutlinerWindow.h"

#include <Drawable.h>
#include <GradientSky.h>
#include <Surface.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {

    OverlayTopBar::OverlayTopBar(EditorViewport* owner) : OverlayUI(owner) {}

    void OverlayTopBar::ShowAddMenuPopup()
    {
      EditorScenePtr currScene = g_app->GetCurrentScene();
      Entity* createdEntity    = nullptr;
      if (ImGui::BeginMenu("Mesh"))
      {
        if (ImGui::MenuItem(ICON_FA_CUBE " Cube"))
        {
          createdEntity = MakeNew<Cube>();
          createdEntity->GetMeshComponent()->Init(false);
        }
        if (ImGui::MenuItem(ICON_FA_CIRCLE " Sphere"))
        {
          createdEntity = MakeNew<Sphere>();
          createdEntity->GetMeshComponent()->Init(false);
        }
        if (ImGui::MenuItem(ICON_FA_CARET_UP " Cone"))
        {
          Cone* cone = MakeNew<Cone>();
          cone->Generate(1.0f, 1.0f, 30, 30);
          createdEntity = cone;
          createdEntity->GetMeshComponent()->Init(false);
        }
        if (ImGui::MenuItem(ICON_FA_SQUARE " Plane"))
        {
          createdEntity = MakeNew<Quad>();
          createdEntity->GetMeshComponent()->Init(false);
        }
        if (ImGui::MenuItem(ICON_FA_GITHUB_ALT " Monkey"))
        {
          Drawable* suzanne = MakeNew<Drawable>();
          suzanne->SetMesh(GetMeshManager()->Create<Mesh>(MeshPath("suzanne.mesh", true)));
          suzanne->GetMesh()->Init(false);
          createdEntity = suzanne;
        }
        ImGui::EndMenu();
      }
      ImGui::Separator();

      if (ImGui::BeginMenu("2D UI"))
      {
        if (ImGui::MenuItem("Surface"))
        {
          Surface* srf = MakeNew<Surface>();
          srf->SetSizeVal(Vec2(100.0f, 30.0f));
          createdEntity = srf;
          createdEntity->GetMeshComponent()->Init(false);
        }

        if (ImGui::MenuItem("Button"))
        {
          Button* btn = MakeNew<Button>();
          btn->Update(Vec2(100.0f, 30.0f), Vec2(0.5f, 0.5f));
          createdEntity = btn;
          createdEntity->GetMeshComponent()->Init(false);
        }
        ImGui::EndMenu();
      }

      ImGui::Separator();
      if (ImGui::MenuItem(ICON_FA_ARROWS " Node"))
      {
        createdEntity = GetEntityFactory()->CreateByType(EntityType::Entity_Node);
      }

      if (ImGui::MenuItem(ICON_FA_VIDEO_CAMERA " Camera"))
      {
        createdEntity = MakeNew<EditorCamera>();
      }

      if (ImGui::BeginMenu(ICON_FA_LIGHTBULB " Light"))
      {
        if (ImGui::MenuItem(ICON_FA_SUN " Directional"))
        {
          EditorDirectionalLight* light = MakeNew<EditorDirectionalLight>();
          light->Init();
          createdEntity = static_cast<Entity*>(light);
        }

        if (ImGui::MenuItem(ICON_FA_LIGHTBULB " Point"))
        {
          EditorPointLight* light = MakeNew<EditorPointLight>();
          light->Init();
          createdEntity = static_cast<Entity*>(light);
        }

        if (ImGui::MenuItem(ICON_FA_LIGHTBULB " Spot"))
        {
          EditorSpotLight* light = MakeNew<EditorSpotLight>();
          light->Init();
          createdEntity = static_cast<Entity*>(light);
        }

        if (ImGui::MenuItem(ICON_FA_CLOUD " Sky"))
        {
          createdEntity = MakeNew<Sky>();
        }

        if (ImGui::MenuItem(ICON_FA_SKYATLAS " Gradient Sky"))
        {
          createdEntity = MakeNew<GradientSky>();
        }

        ImGui::EndMenu();
      }

      if (createdEntity != nullptr)
      {
        const auto isSameTypeFn = [createdEntity](const Entity* e) -> bool
        { return e->GetType() == createdEntity->GetType(); };

        String typeName                   = EntityTypeToString(createdEntity->GetType());
        const EntityRawPtrArray& entities = currScene->GetEntities();
        size_t numSameType                = std::count_if(entities.cbegin(), entities.cend(), isSameTypeFn);

        String suffix                     = numSameType == 0 ? "" : "_" + std::to_string(numSameType);

        // if numSameType equals 0 EntityType otherwise EntityType_123
        createdEntity->SetNameVal(typeName + suffix);
        currScene->AddEntity(createdEntity);

        if (OutlinerWindow* outliner = g_app->GetOutliner())
        {
          if (outliner->IsInsertingAtTheEndOfEntities())
          {
            outliner->Focus(createdEntity);
          }
          // if right clicked this will try to insert to where we clicked
          // otherwise(top bar add) this will spawn at the end of the list.
          outliner->TryReorderEntites({createdEntity});
          currScene->ValidateBillboard(createdEntity);
        }
      }
    }

    void OverlayTopBar::Show()
    {
      assert(m_owner);
      if (m_owner == nullptr)
      {
        return;
      }

      ImVec2 overlaySize(360, 30);

      // Center the toolbar.
      float width  = ImGui::GetWindowContentRegionWidth();
      float offset = (width - overlaySize.x) * 0.5f;
      ImGui::SameLine(offset);

      ImGui::SetNextWindowBgAlpha(0.65f);
      if (ImGui::BeginChildFrame(ImGui::GetID("ViewportOptions"),
                                 overlaySize,
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoScrollWithMouse))
      {
        SetOwnerState();

        ImGui::BeginTable("##SettingsBar", 8, ImGuiTableFlags_SizingStretchProp);
        ImGui::TableNextRow();

        uint nextItemIndex = 0;

        ShowAddMenu(ShowAddMenuPopup, nextItemIndex);

        CameraAlignmentOptions(nextItemIndex);

        ShowTransformOrientation(nextItemIndex);

        SnapOptions(nextItemIndex);

        ImGui::EndTable();
      }
      ImGui::EndChildFrame();
    }

    void OverlayTopBar::ShowAddMenu(std::function<void()> showMenuFn, uint32_t& nextItemIndex)
    {
      ImGui::TableSetColumnIndex(nextItemIndex++);
      // Get the current cursor position
      ImVec2 cursor_pos = ImGui::GetCursorPos();
      // Set the new cursor position with an offset
      ImGui::SetCursorPos(ImVec2(cursor_pos.x + 3.0f, cursor_pos.y + 3.0f));
      ImGui::Text(ICON_FA_GLOBE);
      // Reset the cursor position to the default value
      ImGui::SetCursorPos(cursor_pos);

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
      g_app->m_snapsEnabled = UI::ToggleButton(UI::m_snapIcon->m_textureId, ImVec2(16, 16), g_app->m_snapsEnabled);
      UI::HelpMarker(TKLoc + m_owner->m_name, "Grid snaping\nRight click for options");

      if (ImGui::BeginPopupContextItem("##SnapMenu"))
      {
        ImGui::PushItemWidth(75);
        ImGui::InputFloat("Move delta", &m_owner->m_snapDeltas.x, 0.0f, 0.0f, "%.2f");
        ImGui::InputFloat("Rotate delta", &m_owner->m_snapDeltas.y, 0.0f, 0.0f, "%.2f");
        ImGui::InputFloat("Scale delta", &m_owner->m_snapDeltas.z, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::EndPopup();
      }
    }

    void OverlayTopBar::CameraAlignmentOptions(uint32_t& nextItemIndex)
    {
      bool change = false;
      ImGui::TableSetColumnIndex(nextItemIndex++);

      ImGui::Text(ICON_FA_VIDEO_CAMERA);

      ImGui::TableSetColumnIndex(nextItemIndex++);
      m_owner->m_orbitLock = UI::ToggleButton(m_owner->m_orbitLock ? ICON_FA_LOCK : ICON_FA_UNLOCK,
                                              ImVec2(20.0f, 20.0f),
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
      if (ImGui::BeginCombo("##VC", itemsCam[currentItemCam], ImGuiComboFlags_None))
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
            g_app->GetConsole()->AddLog("No camera selected.\nSelect a camera from the scene.", LogType::Error);
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
            String cmd                 = "SetCameraTransform --v \"" + m_owner->m_name + "\" " + view;
            g_app->GetConsole()->ExecCommand(cmd);
          }
        }
      }
      UI::HelpMarker(TKLoc + m_owner->m_name, "Camera Orientation\n");
    }

  } // namespace Editor
} // namespace ToolKit