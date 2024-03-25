/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "PropInspectorWindow.h"

#include "App.h"
#include "ComponentView.h"
#include "CustomDataView.h"
#include "EntityView.h"
#include "MaterialView.h"
#include "MeshView.h"
#include "PrefabView.h"

#include <Camera.h>
#include <DirectionComponent.h>
#include <FileManager.h>
#include <GradientSky.h>
#include <Material.h>
#include <Mesh.h>
#include <Prefab.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {

    TKDefineClass(PropInspectorWindow, Window);

    PropInspectorWindow::PropInspectorWindow()
    {
      // order is important, depends on enum viewType
      m_views.resize((uint) ViewType::ViewCount);
      m_views[(uint) ViewType::Entity]     = new EntityView();
      m_views[(uint) ViewType::CustomData] = new CustomDataView();
      m_views[(uint) ViewType::Component]  = new ComponentView();
      m_views[(uint) ViewType::Material]   = new MaterialView();
      m_views[(uint) ViewType::Mesh]       = new MeshView();
      m_views[(uint) ViewType::Prefab]     = new PrefabView();

      m_entityViews.push_back((uint) ViewType::Entity);
      m_entityViews.push_back((uint) ViewType::CustomData);
      m_entityViews.push_back((uint) ViewType::Component);
      m_entityViews.push_back((uint) ViewType::Material);
      m_entityViews.push_back((uint) ViewType::Mesh);

      // prefab view doesn't have CustomDataView or ComponentView
      // because prefab view provides this views. hence no need to add views
      m_prefabViews.push_back((uint) ViewType::Entity);
      m_prefabViews.push_back((uint) ViewType::Prefab);
      m_prefabViews.push_back((uint) ViewType::Material);
      m_prefabViews.push_back((uint) ViewType::Mesh);
    }

    PropInspectorWindow::PropInspectorWindow(XmlNode* node) : PropInspectorWindow() { DeSerialize(SerializationFileInfo(), node); }

    PropInspectorWindow::~PropInspectorWindow()
    {
      for (ViewRawPtr& view : m_views)
      {
        SafeDel(view);
      }
    }

    void PropInspectorWindow::SetActiveView(ViewType viewType) { m_activeView = viewType; }

    MaterialView* PropInspectorWindow::GetMaterialView()
    {
      return static_cast<MaterialView*>(m_views[(uint) ViewType::Material]);
    }

    void PropInspectorWindow::DeterminateSelectedMaterial(EntityPtr curEntity)
    {
      // if material view is active. determinate selected material
      MaterialView* matView = dynamic_cast<MaterialView*>(m_views[(uint) m_activeView]);
      if (matView == nullptr)
      {
        return;
      }

      if (curEntity == nullptr)
      {
        // set empty array, there is no material sellected
        matView->SetMaterials({});
        return;
      }

      if (curEntity->IsA<Prefab>())
      {
        PrefabView* prefView = static_cast<PrefabView*>(m_views[(uint) ViewType::Prefab]);
        if (EntityPtr prefEntity = prefView->GetActiveEntity())
        {
          curEntity = prefEntity;
        }
      }

      if (MaterialComponentPtr mat = curEntity->GetMaterialComponent())
      {
        matView->SetMaterials(mat->GetMaterialList());
      }
      else
      {
        // entity doesn't have material clear material view.
        matView->SetMaterials({});
      }
    }

    void PropInspectorWindow::Show()
    {
      ImVec4 windowBg  = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
      ImVec4 childBg   = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
      ImGuiStyle style = ImGui::GetStyle();
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(3.0f, 0.0f));
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, style.ItemSpacing.y));

      EntityPtr curEntity = g_app->GetCurrentScene()->GetCurrentSelection();
      bool isPrefab       = curEntity != nullptr && curEntity->IsA<Prefab>();

      UIntArray views     = isPrefab ? m_prefabViews : m_entityViews;

      if (ImGui::Begin(m_name.c_str(), &m_visible, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
      {
        HandleStates();

        const ImVec2 windowSize      = ImGui::GetWindowSize();
        const ImVec2 sidebarIconSize = ImVec2(18.0f, 18.0f);

        // Show ViewType sidebar
        ImGui::GetStyle()            = style;
        const ImVec2 spacing         = ImGui::GetStyle().ItemSpacing;
        const ImVec2 sidebarSize     = ImVec2(spacing.x + sidebarIconSize.x + spacing.x, windowSize.y);

        DeterminateSelectedMaterial(curEntity);

        if (ImGui::BeginChildFrame(ImGui::GetID("ViewTypeSidebar"), sidebarSize, 0))
        {
          for (uint viewIndx : views)
          {
            ViewRawPtr view   = m_views[viewIndx];
            bool isActiveView = (uint) m_activeView == viewIndx;

            ImGui::PushStyleColor(ImGuiCol_Button, isActiveView ? windowBg : childBg);

            // is this font icon ? (prefab's icon is font icon)
            if (view->m_fontIcon.size() > 0)
            {
              ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 1.0, 1.0));

              if (ImGui::Button(view->m_fontIcon.data(), ImVec2(27, 25)))
              {
                m_activeView = (ViewType) viewIndx;
              }
              ImGui::PopStyleColor();
            }
            else if (ImGui::ImageButton(Convert2ImGuiTexture(view->m_viewIcn), sidebarIconSize))
            {
              m_activeView = (ViewType) viewIndx;
            }
            UI::HelpMarker("View#" + std::to_string(viewIndx), view->m_viewName.data());
            ImGui::PopStyleColor(1);
          }
        }
        ImGui::EndChildFrame();

        ImGui::SameLine();

        if (ImGui::BeginChild(ImGui::GetID("PropInspectorActiveView"), ImGui::GetContentRegionAvail()))
        {
          m_views[(uint) m_activeView]->Show();
        }
        ImGui::EndChild();
      }
      ImGui::End();
      ImGui::PopStyleVar(2);
    }

    void PropInspectorWindow::DispatchSignals() const { ModShortCutSignals(); }

    void PropInspectorWindow::SetMaterials(const MaterialPtrArray& mat)
    {
      m_activeView          = ViewType::Material;
      uint matViewIndx      = (uint) ViewType::Material;
      MaterialView* matView = (MaterialView*) m_views[matViewIndx];
      matView->SetMaterials(mat);
    }

    void PropInspectorWindow::SetMeshView(MeshPtr mesh)
    {
      m_activeView       = ViewType::Mesh;
      uint meshViewIndx  = (uint) ViewType::Mesh;
      MeshView* meshView = (MeshView*) m_views[meshViewIndx];
      meshView->SetMesh(mesh);
    }

  } // namespace Editor
} // namespace ToolKit
