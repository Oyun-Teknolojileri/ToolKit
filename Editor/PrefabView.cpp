#include "PrefabView.h"
#include "CustomDataView.h"
#include "ComponentView.h"
#include "PrefabView.h"
#include "Global.h"
#include "App.h"
#include <Prefab.h>

namespace ToolKit
{
  namespace Editor
  {
    // PrefabView
    //////////////////////////////////////////////////////////////////////////

    PrefabView::PrefabView() : View("Prefab View")
    {
      m_viewID  = 2;
      m_viewIcn = UI::m_prefabIcn;
    }

    PrefabView::~PrefabView()
    {
    }

    bool PrefabView::DrawHeader(Entity* ntt, ImGuiTreeNodeFlags flags)
    {
      const String sId = "##" + std::to_string(ntt->GetIdVal());
      if (m_activeChildEntity == ntt)
      {
        flags |= ImGuiTreeNodeFlags_Selected;
      }
      bool isOpen = ImGui::TreeNodeEx(sId.c_str(), flags);
      if (ImGui::IsItemClicked())
      {
        m_activeChildEntity = ntt;
      }

      TexturePtr icon  = nullptr;
      EntityType eType = ntt->GetType();
      switch (eType)
      {
      case EntityType::Entity_Node:
        icon = UI::m_arrowsIcon;
        break;
      case EntityType::Entity_Prefab:
        icon = UI::m_prefabIcn;
        break;
      }

      if (icon)
      {
        ImGui::SameLine();
        ImGui::Image(Convert2ImGuiTexture(icon), ImVec2(20.0f, 20.0f));
      }

      ImGui::SameLine();
      ImGui::Text(ntt->GetNameVal().c_str());

      // Hiearchy visibility
      float offset = ImGui::GetContentRegionAvail().x - 40.0f;
      ImGui::SameLine(offset);
      icon = ntt->GetVisibleVal() ? UI::m_visibleIcon : UI::m_invisibleIcon;

      // Texture only toggle button.
      ImGui::PushID(static_cast<int>(ntt->GetIdVal()));
      if (UI::ImageButtonDecorless(
              icon->m_textureId, ImVec2(15.0f, 15.0f), false))
      {
        ntt->SetVisibility(!ntt->GetVisibleVal(), true);
      }

      ImGui::PopID();

      return isOpen;
    }

    void PrefabView::ShowNode(Entity* e)
    {
      ImGuiTreeNodeFlags nodeFlags = g_treeNodeFlags;

      if (e->m_node->m_children.empty() ||
          e->GetType() == EntityType::Entity_Prefab)
      {
        nodeFlags |=
            ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        DrawHeader(e, nodeFlags);
      }
      else
      {
        if (DrawHeader(e, nodeFlags))
        {
          for (Node* n : e->m_node->m_children)
          {
            Entity* childNtt = n->m_entity;
            if (childNtt != nullptr)
            {
              ShowNode(childNtt);
            }
          }

          ImGui::TreePop();
        }
      }
    }

    void PrefabView::Show()
    {
      Entity* cur = g_app->GetCurrentScene()->GetCurrentSelection();
      if (cur != m_entity)
      {
        m_entity            = cur;
        m_activeChildEntity = nullptr;
      }
      if (m_entity == nullptr || Prefab::GetPrefabRoot(m_entity) == nullptr)
      {
        ImGui::Text("Select a prefab entity");
        return;
      }

      // Display scene hierarchy
      if (ImGui::CollapsingHeader("Prefab Scene View",
                                  ImGuiTreeNodeFlags_DefaultOpen))
      {
        if (ImGui::BeginChild("##Prefab Scene Nodes", ImVec2(0, 200), true))
        {
          if (DrawHeader(m_entity,
                         g_treeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen))
          {
            for (Node* n : m_entity->m_node->m_children)
            {
              if (n->m_entity)
              {
                ShowNode(n->m_entity);
              }
            }

            ImGui::TreePop();
          }
        }
        ImGui::EndChild();
      }

      Entity* shownEntity = m_entity;
      if (m_activeChildEntity)
      {
        shownEntity = m_activeChildEntity;
      }

      ParameterVariantRawPtrArray inheritedParams;
      shownEntity->m_localData.GetByCategory(CustomDataCategory.Name,
                                             inheritedParams);
      CustomDataView::ShowCustomData(shownEntity, "Custom Data##1", inheritedParams, false);

      if (ImGui::CollapsingHeader("Components##1",
                                  ImGuiTreeNodeFlags_DefaultOpen))
      {
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, g_indentSpacing);

        std::vector<ULongID> compRemove;
        for (ComponentPtr& com : shownEntity->GetComponentPtrArray())
        {
          ComponentView::ShowComponentBlock(com, false);
        }

        ImGui::PopStyleVar();
      }
    }

  }
}