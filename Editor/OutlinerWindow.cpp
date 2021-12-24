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

    void OutlinerWindow::ShowNode(Entity* e)
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

      if (e->m_node->m_children.empty())
      {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
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
              if (childNtt->m_node->m_children.empty())
              {
                nodeFlags = baseFlags;
                if (g_app->m_scene->IsSelected(childNtt->m_id))
                {
                  nodeFlags |= ImGuiTreeNodeFlags_Selected;
                }

                nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                DrawHeader(childNtt, nodeFlags);
              }
              else
              {
                nodeFlags = baseFlags;
                if (g_app->m_scene->IsSelected(childNtt->m_id))
                {
                  nodeFlags |= ImGuiTreeNodeFlags_Selected;
                }

                if (DrawHeader(childNtt, nodeFlags))
                {
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
              }
            }
          }

          ImGui::TreePop();
        }
      }
    }

    void OutlinerWindow::SetItemState(Entity* e)
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
    }

    void OutlinerWindow::Show()
    {
      ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 4.0f);

      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        HandleStates();

        g_parent = NULL_ENTITY;
        g_child = NULL_ENTITY;

        if (DrawHeader("Scene", 0, ImGuiTreeNodeFlags_DefaultOpen, UI::m_collectionIcon))
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

      ImGui::PopStyleVar();
      ImGui::End();
    }

    Window::Type OutlinerWindow::GetType() const
    {
      return Window::Type::Outliner;
    }

    void OutlinerWindow::DispatchSignals() const
    {
      ModShortCutSignals();
    }

    bool OutlinerWindow::DrawHeader(const String& text, uint id, ImGuiTreeNodeFlags flags, TexturePtr icon)
    {
      const String sId = "##" + std::to_string(id);
      bool isOpen = ImGui::TreeNodeEx(sId.c_str(), flags);

      if (icon)
      {
        ImGui::SameLine();
        ImGui::Image(Convert2ImGuiTexture(icon), ImVec2(20.0f, 20.0f));
      }

      ImGui::SameLine();
      ImGui::Text(text.c_str());

      return isOpen;
    }

    bool OutlinerWindow::DrawHeader(Entity* ntt, ImGuiTreeNodeFlags flags)
    {
      const String sId = "##" + std::to_string(ntt->m_id);
      bool isOpen = ImGui::TreeNodeEx(sId.c_str(), flags);

      SetItemState(ntt);

      TexturePtr icon = nullptr;
      EntityType eType = ntt->GetType();
      if (eType == EntityType::Entity_Node)
      {
        if (ntt->m_id == g_2dLayerId || ntt->m_id == g_3dLayerId)
        {
          icon = UI::m_collectionIcon;
        }
        else
        {
          icon = UI::m_arrowsIcon;
        }
      }

      if (icon)
      {
        ImGui::SameLine();
        ImGui::Image(Convert2ImGuiTexture(icon), ImVec2(20.0f, 20.0f));
      }

      ImGui::SameLine();
      ImGui::Text(ntt->m_name.c_str());

      return isOpen;
    }

  }
}