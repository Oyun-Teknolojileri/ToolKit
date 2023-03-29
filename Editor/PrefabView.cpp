#include "PrefabView.h"

#include "App.h"
#include "ComponentView.h"
#include "CustomDataView.h"
#include "Global.h"
#include "PrefabView.h"

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

    PrefabView::~PrefabView() {}

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
      
      // show name, open and slash eye, lock and unlock.
      UI::ShowEntityTreeNodeContent(ntt);
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
      CustomDataView::ShowCustomData(shownEntity,
                                     "Custom Data##1",
                                     inheritedParams,
                                     false);

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

  } // namespace Editor
} // namespace ToolKit