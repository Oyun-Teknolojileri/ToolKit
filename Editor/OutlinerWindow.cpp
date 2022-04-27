#include "OutlinerWindow.h"
#include "GlobalDef.h"
#include "Mod.h"
#include "Util.h"
#include "FolderWindow.h"
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
    ULongID g_parent = NULL_HANDLE;
    std::vector<ULongID> g_child;
    static ImGuiTreeNodeFlags g_baseNodeFlags
      = ImGuiTreeNodeFlags_OpenOnArrow
      | ImGuiTreeNodeFlags_OpenOnDoubleClick
      | ImGuiTreeNodeFlags_SpanAvailWidth
      | ImGuiTreeNodeFlags_AllowItemOverlap
      | ImGuiTreeNodeFlags_FramePadding;

    void OutlinerWindow::ShowNode(Entity* e)
    {
      ImGuiTreeNodeFlags nodeFlags = g_baseNodeFlags;
      EditorScenePtr currScene = g_app->GetCurrentScene();
      if (currScene->IsSelected(e->Id()))
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
                nodeFlags = g_baseNodeFlags;
                if (currScene->IsSelected(childNtt->Id()))
                {
                  nodeFlags |= ImGuiTreeNodeFlags_Selected;
                }

                nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                DrawHeader(childNtt, nodeFlags);
              }
              else
              {
                nodeFlags = g_baseNodeFlags;
                if (currScene->IsSelected(childNtt->Id()))
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
      EditorScenePtr currScene = g_app->GetCurrentScene();

      if (ImGui::IsItemClicked())
      {
        if (ImGui::GetIO().KeyShift)
        {
          if (currScene->IsSelected(e->Id()))
          {
            currScene->RemoveFromSelection(e->Id());
          }
          else
          {
            currScene->AddToSelection(e->Id(), true);
          }
        }
        else
        {
          if (!currScene->IsSelected(e->Id()))
          {
            currScene->AddToSelection(e->Id(), false);
          }
        }
      }

      if (ImGui::BeginDragDropSource())
      {
        ImGui::SetDragDropPayload("HierarcyChange", nullptr, 0);
        ImGui::Text("Drop on the new parent.");
        ImGui::EndDragDropSource();
      }

      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HierarcyChange"))
        {
          // Change the selected files hierarchy
          EntityRawPtrArray selected;
          currScene->GetSelectedEntities(selected);

          for (int i = 0; i < selected.size(); i++)
          {
            if (selected[i]->Id() != e->Id())
            {
              g_child.push_back(selected[i]->Id());
            }
          }
          g_parent = e->Id();
        }
        ImGui::EndDragDropTarget();
      }
    }

    void OutlinerWindow::Show()
    {
      EditorScenePtr currScene = g_app->GetCurrentScene();
      ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 6.0f);
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        HandleStates();

        if (DrawRootHeader("Scene", 0, g_baseNodeFlags | ImGuiTreeNodeFlags_DefaultOpen, UI::m_collectionIcon))
        {
          const EntityRawPtrArray& ntties = currScene->GetEntities();
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
      std::vector<ULongID>::iterator it = g_child.begin();
      while (it != g_child.end())
      {
        Entity* child = currScene->GetEntity(*it);
        child->m_node->OrphanSelf(true);

        if (g_parent != NULL_HANDLE)
        {
          Entity* parent = currScene->GetEntity(g_parent);
          parent->m_node->AddChild(child->m_node, true);
        }
        it = g_child.erase(it);
      }

      g_parent = NULL_HANDLE;

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

    void OutlinerWindow::Focus(Entity* ntt)
    {
      m_nttFocusPath.push_back(ntt);
      GetParents(ntt, m_nttFocusPath);
    }

    bool OutlinerWindow::DrawRootHeader(const String& rootName, uint id, ImGuiTreeNodeFlags flags, TexturePtr icon)
    {
      const String sId = "##" + std::to_string(id);
      bool isOpen = ImGui::TreeNodeEx(sId.c_str(), flags);

      // Orphan in this case.
      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HierarcyChange"))
        {
          EntityRawPtrArray selected;
          EditorScenePtr currScene = g_app->GetCurrentScene();
          currScene->GetSelectedEntities(selected);

          for (int i = 0; i < selected.size(); i++)
          {
            if (selected[i]->Id() != NULL_HANDLE)
            {
              g_child.push_back(selected[i]->Id());
            }
          }
          g_parent = NULL_HANDLE;
        }
        ImGui::EndDragDropTarget();
      }

      if (icon)
      {
        ImGui::SameLine();
        ImGui::Image(Convert2ImGuiTexture(icon), ImVec2(20.0f, 20.0f));
      }

      ImGui::SameLine();
      ImGui::Text(rootName.c_str());

      return isOpen;
    }

    bool OutlinerWindow::DrawHeader(Entity* ntt, ImGuiTreeNodeFlags flags)
    {
      bool focusToItem = false;
      if (int size = (int)m_nttFocusPath.size())
      {
        int focusIndx = IndexOf(ntt, m_nttFocusPath);
        if (focusIndx != -1)
        {
          ImGui::SetNextItemOpen(true);
        }

        focusToItem = focusIndx == 0;
      }

      const String sId = "##" + std::to_string(ntt->Id());
      bool isOpen = ImGui::TreeNodeEx(sId.c_str(), flags);

      if (ImGui::BeginPopupContextItem())
      {
        if (ImGui::MenuItem("SaveAsPrefab"))
        {
          GetSceneManager()->GetCurrentScene()->SavePrefab(ntt);
          if (FolderWindow* browser = g_app->GetAssetBrowser())
          {
            String folderPath, fullPath = PrefabPath("");
            DecomposePath(fullPath, &folderPath, nullptr, nullptr);

            int indx = browser->Exist(folderPath);
            if (indx != -1)
            {
              FolderView& view = browser->GetView(indx);
              view.Refresh();
            }
          }
          ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
      }

      if (focusToItem)
      {
        ImGui::SetScrollHereY();
        m_nttFocusPath.clear();
      }

      SetItemState(ntt);

      TexturePtr icon = nullptr;
      EntityType eType = ntt->GetType();
      if (eType == EntityType::Entity_Node)
      {
        icon = UI::m_arrowsIcon;
      }

      if (icon)
      {
        ImGui::SameLine();
        ImGui::Image(Convert2ImGuiTexture(icon), ImVec2(20.0f, 20.0f));
      }

      ImGui::SameLine();
      ImGui::Text(ntt->Name().c_str());

      // Hiearchy visibility
      float offset = ImGui::GetContentRegionAvail().x - 30.0f;
      ImGui::SameLine(offset);
      icon = ntt->Visible() ? UI::m_visibleIcon : UI::m_invisibleIcon;

      // Texture only toggle button.
      ImGui::PushID(ntt->Id());
      if (UI::ImageButtonDecorless(icon->m_textureId, ImVec2(15.0f, 15.0f), false))
      {
        ntt->SetVisibility(!ntt->Visible(), true);
      }
      ImGui::PopID();

      offset = ImGui::GetContentRegionAvail().x - 10.0f;
      ImGui::SameLine(offset);
      icon = ntt->TransformLock() ? UI::m_lockedIcon : UI::m_unlockedIcon;

      // Texture only toggle button.
      ImGui::PushID(ntt->Id());
      if (UI::ImageButtonDecorless(icon->m_textureId, ImVec2(15.0f, 15.0f), false))
      {
        ntt->SetTransformLock(!ntt->TransformLock(), true);
      }

      ImGui::PopID();

      return isOpen;
    }

  }
}