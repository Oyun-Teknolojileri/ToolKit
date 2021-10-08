#include "stdafx.h"
#include "OutlinerWindow.h"
#include "GlobalDef.h"
#include "Mod.h"
#include "Util.h"
#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {
    OutlinerWindow::OutlinerWindow(XmlNode* node)
    {
      DeSerialize(nullptr, node);
    }

    OutlinerWindow::OutlinerWindow()
    {
    }

    OutlinerWindow::~OutlinerWindow()
    {
    }

    // Recursively show entity hierarchy & update via drag drop.
    EntityId g_parent = NULL_ENTITY;
    EntityId g_child = NULL_ENTITY;
    void ShowNode(Entity* e)
    {
      static ImGuiTreeNodeFlags baseFlags
        = ImGuiTreeNodeFlags_OpenOnArrow
        | ImGuiTreeNodeFlags_OpenOnDoubleClick
        | ImGuiTreeNodeFlags_SpanAvailWidth;

      ImGuiTreeNodeFlags nodeFlags = baseFlags;
      if (g_app->m_scene->IsSelected(e->m_id))
      {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
      }

      auto SetItemStateFn = [](Entity* e) -> void
      {
        if (ImGui::IsItemClicked())
        {
          if (ImGui::GetIO().KeyShift)
          {
            if (g_app->m_scene->IsSelected(e->m_id))
            {
              g_app->m_scene->RemoveFromSelection(e->m_id);
            }
            else
            {
              g_app->m_scene->AddToSelection(e->m_id, true);
            }
          }
          else
          {
            g_app->m_scene->ClearSelection();
            g_app->m_scene->AddToSelection(e->m_id, false);
          }
        }

        if (ImGui::BeginDragDropSource())
        {
          ImGui::SetDragDropPayload("HierarcyChange", &e->m_id, sizeof(EntityId*));
          ImGui::Text("Drop on the new parent.");
          ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
          if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HierarcyChange"))
          {
            IM_ASSERT(payload->DataSize == sizeof(EntityId*));
            g_child = *(EntityId*)payload->Data;
            g_parent = e->m_id;
          }
          ImGui::EndDragDropTarget();
        }
      };

      if (e->m_node->m_children.empty())
      {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        ImGui::TreeNodeEx(e->m_name.c_str(), nodeFlags);
        SetItemStateFn(e);
      }
      else
      {
        if (ImGui::TreeNodeEx(e->m_name.c_str(), nodeFlags))
        {
          SetItemStateFn(e);
          for (Node* n : e->m_node->m_children)
          {
            Entity* childNtt = n->m_entity;
            if (childNtt != nullptr)
            {
              if (childNtt->m_node->m_children.empty())
              {
                nodeFlags = baseFlags;
                if (g_app->m_scene->IsSelected(childNtt->m_id))
                {
                  nodeFlags |= ImGuiTreeNodeFlags_Selected;
                }

                nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                ImGui::TreeNodeEx(childNtt->m_name.c_str(), nodeFlags);
                SetItemStateFn(childNtt);
              }
              else
              {
                nodeFlags = baseFlags;
                if (g_app->m_scene->IsSelected(childNtt->m_id))
                {
                  nodeFlags |= ImGuiTreeNodeFlags_Selected;
                }

                if (ImGui::TreeNodeEx(childNtt->m_name.c_str(), nodeFlags))
                {
                  SetItemStateFn(childNtt);
                  for (Node* deepChildNode : childNtt->m_node->m_children)
                  {
                    Entity* deepChild = deepChildNode->m_entity;
                    if (deepChild)
                    {
                      ShowNode(deepChild);
                    }
                  }

                  ImGui::TreePop();
                }
                else
                {
                  SetItemStateFn(childNtt);
                }
              }
            }
          }

          ImGui::TreePop();
        }
        else
        {
          SetItemStateFn(e);
        }
      }
    }

    void OutlinerWindow::Show()
    {
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        HandleStates();

        g_parent = NULL_ENTITY;
        g_child = NULL_ENTITY;

        if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen))
        {
          // Orphan in this case.
          if (ImGui::BeginDragDropTarget())
          {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HierarcyChange"))
            {
              IM_ASSERT(payload->DataSize == sizeof(EntityId*));
              g_child = *(EntityId*)payload->Data;
              Entity* child = g_app->m_scene->GetEntity(g_child);
              child->m_node->OrphanSelf(true);
            }
            ImGui::EndDragDropTarget();
          }

          const EntityRawPtrArray& ntties = g_app->m_scene->GetEntities();
          EntityRawPtrArray roots;
          GetRootEntities(ntties, roots);

          ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());

          for (Entity* e : roots)
          {
            ShowNode(e);
          }
          ImGui::TreePop();
        }
      }

      // Update hierarchy if there is a change.
      if (g_child != NULL_ENTITY)
      {
        Entity* child = g_app->m_scene->GetEntity(g_child);
        child->m_node->OrphanSelf(true);
        if (g_parent != NULL_ENTITY)
        {
          Entity* parent = g_app->m_scene->GetEntity(g_parent);
          parent->m_node->AddChild(child->m_node, true);
        }
      }

      ImGui::End();
    }

    Window::Type OutlinerWindow::GetType() const
    {
      return Window::Type::Outliner;
    }

    void OutlinerWindow::DispatchSignals() const
    {
      ImGuiIO& io = ImGui::GetIO();

      if (io.KeysDown[io.KeyMap[ImGuiKey_Delete]])
      {
        if (io.KeysDownDuration[io.KeyMap[ImGuiKey_Delete]] == 0.0f)
        {
          ModManager::GetInstance()->DispatchSignal(BaseMod::m_delete);
        }
      }
    }

  }
}