/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "PrefabView.h"

#include "App.h"
#include "ComponentView.h"
#include "CustomDataView.h"
#include "Global.h"
#include "IconsFontAwesome.h"
#include "Prefab.h"

namespace ToolKit
{
  namespace Editor
  {
    // PrefabView
    //////////////////////////////////////////////////////////////////////////

    PrefabView::PrefabView() : View("Prefab View")
    {
      m_viewID   = 2;
      m_viewIcn  = UI::m_prefabIcn;
      m_fontIcon = ICON_FA_CUBES;
    }

    PrefabView::~PrefabView() {}

    bool PrefabView::HasActiveEntity() const { return m_activeChildEntity != nullptr; }

    EntityPtr PrefabView::GetActiveEntity() { return m_activeChildEntity; }

    bool PrefabView::DrawHeader(EntityPtr ntt, ImGuiTreeNodeFlags flags)
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

    void PrefabView::ShowNode(EntityPtr ntt)
    {
      ImGuiTreeNodeFlags nodeFlags = g_treeNodeFlags;

      if (ntt->m_node->m_children.empty() || ntt->IsA<Prefab>())
      {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        DrawHeader(ntt, nodeFlags);
      }
      else
      {
        if (DrawHeader(ntt, nodeFlags))
        {
          for (Node* node : ntt->m_node->m_children)
          {
            if (EntityPtr childNtt = node->OwnerEntity())
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
      EntityPtr ntt = m_entity.lock();
      EntityPtr cur = g_app->GetCurrentScene()->GetCurrentSelection();
      if (cur != ntt)
      {
        m_entity            = cur;
        m_activeChildEntity = nullptr;
      }

      if (ntt == nullptr || Prefab::GetPrefabRoot(ntt) == nullptr)
      {
        ImGui::Text("Select a prefab entity");
        return;
      }

      // Display scene hierarchy
      if (ImGui::CollapsingHeader("Prefab Scene View", ImGuiTreeNodeFlags_DefaultOpen))
      {
        if (ImGui::BeginChild("##Prefab Scene Nodes", ImVec2(0, 200), true))
        {
          if (DrawHeader(ntt, g_treeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen))
          {
            for (Node* node : ntt->m_node->m_children)
            {
              if (EntityPtr childNtt = node->OwnerEntity())
              {
                ShowNode(childNtt);
              }
            }

            ImGui::TreePop();
          }
        }
        ImGui::EndChild();
      }

      EntityPtr shownEntity = ntt;
      if (m_activeChildEntity)
      {
        shownEntity = m_activeChildEntity;
      }

      ParameterVariantRawPtrArray inheritedParams;
      shownEntity->m_localData.GetByCategory(CustomDataCategory.Name, inheritedParams);
      CustomDataView::ShowCustomData(shownEntity, "Custom Data##1", inheritedParams, false);

      if (ImGui::CollapsingHeader("Components##1", ImGuiTreeNodeFlags_DefaultOpen))
      {
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, g_indentSpacing);
        for (ComponentPtr& com : shownEntity->GetComponentPtrArray())
        {
          ComponentView::ShowComponentBlock(com, false);
        }

        ImGui::PopStyleVar();
      }
    }

  } // namespace Editor
} // namespace ToolKit