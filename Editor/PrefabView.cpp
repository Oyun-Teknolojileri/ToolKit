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
          for (Node* n : ntt->m_node->m_children)
          {
            EntityPtr childNtt = n->m_entity;
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
      EntityPtr cur = g_app->GetCurrentScene()->GetCurrentSelection();
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
      if (ImGui::CollapsingHeader("Prefab Scene View", ImGuiTreeNodeFlags_DefaultOpen))
      {
        if (ImGui::BeginChild("##Prefab Scene Nodes", ImVec2(0, 200), true))
        {
          if (DrawHeader(m_entity, g_treeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen))
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

      EntityPtr shownEntity = m_entity;
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