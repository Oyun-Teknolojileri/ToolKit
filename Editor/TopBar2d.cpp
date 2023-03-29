#include "TopBar2d.h"

#include "App.h"
#include "EditorScene.h"
#include "EditorViewport2d.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    Overlay2DTopBar::Overlay2DTopBar(EditorViewport* owner)
        : OverlayTopBar(owner)
    {
    }

    void Overlay2DTopBar::Show()
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
          Surface* suface = new Surface(Vec2(100.0f, 30.0f), Vec2(0.0f, 0.0f));
          suface->GetMeshComponent()->Init(false);
          currScene->AddEntity(suface);
        }

        if (ImGui::MenuItem("Button"))
        {
          Surface* suface = new Button(Vec2(100.0f, 30.0f));
          suface->GetMeshComponent()->Init(false);
          currScene->AddEntity(suface);
        }

        if (ImGui::MenuItem("Canvas"))
        {
          Canvas* canvasPanel = new Canvas(Vec2(800.0f, 600.0f));
          canvasPanel->SetPivotOffsetVal({0.5f, 0.5f});
          canvasPanel->GetMeshComponent()->Init(false);
          currScene->AddEntity(canvasPanel);
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Node"))
        {
          Entity* node =
              GetEntityFactory()->CreateByType(EntityType::Entity_Node);
          currScene->AddEntity(node);
        }
      };

      ImVec2 overlaySize(300, 34);

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

        ShowTransformOrientation(nextItemIndex);

        SnapOptions(nextItemIndex);

        Show2DViewZoomOptions(nextItemIndex);

        ShowGridOptions(nextItemIndex);

        ImGui::EndTable();
      }
      ImGui::EndChildFrame();
    }

    void Overlay2DTopBar::Show2DViewZoomOptions(uint32_t& nextItemIndex)
    {
      EditorViewport2d* editorViewport =
          reinterpret_cast<EditorViewport2d*>(m_owner);
      ImGui::TableSetColumnIndex(nextItemIndex++);
      if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_viewZoomIcon),
                             ImVec2(16, 16)))
      {
        editorViewport->m_zoomPercentage = 100;
      }
      UI::HelpMarker(TKLoc + m_owner->m_name, "Reset Zoom");
      ImGui::TableSetColumnIndex(nextItemIndex++);
      ImGui::Text("%u%%", uint32_t(editorViewport->m_zoomPercentage));
    }

    void Overlay2DTopBar::ShowGridOptions(uint32_t& nextItemIndex)
    {
      auto ShowGridOptionsFn = [this]() -> void
      {
        ImGui::PushItemWidth(75);
        EditorViewport2d* editorViewport =
            reinterpret_cast<EditorViewport2d*>(m_owner);

        static const uint16_t cellSizeStep = 5;
        ImGui::InputScalar("Cell Size",
                           ImGuiDataType_U16,
                           &editorViewport->m_gridCellSizeByPixel,
                           &cellSizeStep);

        editorViewport->m_gridCellSizeByPixel =
            glm::max((editorViewport->m_gridCellSizeByPixel / cellSizeStep) *
                         cellSizeStep,
                     1u);
      };

      ImGui::TableSetColumnIndex(nextItemIndex++);
      if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_gridIcon),
                             ImVec2(18, 18)))
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

  } // namespace Editor
} // namespace ToolKit