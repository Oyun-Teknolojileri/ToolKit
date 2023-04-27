#include "OutlinerWindow.h"

#include "App.h"
#include "FolderWindow.h"
#include "Global.h"
#include "IconsFontAwesome.h"
#include "Mod.h"
#include "UI.h"
#include "Util.h"
#include "imgui_internal.h"

#include <Prefab.h>

#include <stack>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    OutlinerWindow::OutlinerWindow(XmlNode* node)
    {
      DeSerialize(nullptr, node);
    }

    OutlinerWindow::OutlinerWindow() {}

    OutlinerWindow::~OutlinerWindow() {}

    // Recursively show entity hierarchy & update via drag drop.
    ULongID g_parent = NULL_HANDLE;
    std::stack<ULongID> g_reparentQueue;
    
    // returns row height for tree nodes
    inline float GetLineHeight()
    {
      float item_spacing_y = ImGui::GetStyle().ItemSpacing.y;
      return ImGui::GetTextLineHeight() + item_spacing_y;
    }

    void DrawTreeNodeLine(int numNodes, ImVec2 rectMin)
    {
      float line_height = GetLineHeight();
      float halfHeight = line_height * 0.5f;
                       
      ImVec2 cursorPos = ImGui::GetCursorScreenPos();
      // -11 align line with arrow
      rectMin.x        = cursorPos.x - 11.0f;
      rectMin.y       += halfHeight;

      float bottom = rectMin.y + (numNodes * line_height);
      bottom -= (halfHeight + 1.0f); // move up a little

      ImDrawList* drawList = ImGui::GetWindowDrawList();
      const ImColor color  = ImGui::GetColorU32(ImGuiCol_Text);
      drawList->AddLine(rectMin, ImVec2(rectMin.x, bottom), color);
      // a little bulge at the end of the line
      drawList->AddLine(ImVec2(rectMin.x, bottom),
                        ImVec2(rectMin.x + 5.0f, bottom),
                        color);
    }

    // returns total drawed nodes
    int OutlinerWindow::ShowNode(Entity* e, int depth)
    {
      if (m_shownEntities.count(e) == 0)
      {
        return 0;
      }

      ImGuiTreeNodeFlags nodeFlags = g_treeNodeFlags;
      EditorScenePtr currScene     = g_app->GetCurrentScene();
      if (currScene->IsSelected(e->GetIdVal()))
      {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
      }

      int numNodes = 1; // 1 itself and we will sum childs
      m_indexToEntity.push_back(e);

      if (e->m_node->m_children.empty() ||
          e->GetType() == EntityType::Entity_Prefab)
      {
        nodeFlags |=
            ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        DrawHeader(e, nodeFlags, depth);
      }
      else
      {
        if (DrawHeader(e, nodeFlags, depth))
        {
          ImVec2 rectMin = ImGui::GetItemRectMin();

          for (Node* n : e->m_node->m_children)
          {
            numNodes += ShowNode(n->m_entity, depth + 1);
          }

          DrawTreeNodeLine(numNodes, rectMin);
          ImGui::TreePop();
        }
      }
      return numNodes;
    }

    // when we multi select the dragging entities are not sorted 
    // we need to sort dragged entities in order to preserve order when we move the entities
    void OutlinerWindow::SortDraggedEntitiesByNodeIndex()
    {
      std::sort(m_draggingEntities.begin(), m_draggingEntities.end(),
                [this](Entity* a, Entity* b) -> bool
                {
                  return contains(m_indexToEntity, a) < contains(m_indexToEntity, b);
                });
    }

    void OutlinerWindow::SelectEntitiesBetweenNodes(EditorScene* scene,
                                                    Entity* a, Entity* b)
    {
      if (a == b)
      {
        return;
      }

      if (a->m_node->m_parent != b->m_node->m_parent)
      {
        GetLogger()->WriteConsole(LogType::Warning, 
                                  "selected entities should have same parent!");
        return;
      }

      size_t i = 0ull;
      int numFound = 0;
      // one of the parents null means both of them null,
      // so we will search in roots
      if (a->m_node->m_parent == nullptr) 
      {
        // find locations of a and b on m_roots
        for (;i < m_roots.size() && numFound != 2; ++i)
        {
          numFound += (m_roots[i] == a) + (m_roots[i] == b);

          // if we found a or b we will select all of the nodes in bettween them
          if (numFound >= 1ull)
          {
            scene->AddToSelection(m_roots[i]->GetIdVal(), true);
          }
        }
        return;
      }

      // otherwise this means both of the entities has same parent
      // so we will select in between childs

      NodePtrArray& children = a->m_node->m_parent->m_children;
      // find locations of a and b on parents childs
      for (; i < children.size() && numFound != 2; ++i)
      {
        numFound += (children[i] == a->m_node) + (children[i] == b->m_node);

        // if we found a or b we will select all of the nodes in bettween them
        if (numFound >= 1ull)
        {
          scene->AddToSelection(children[i]->m_entity->GetIdVal(), true);
        }
      }
    }

    void OutlinerWindow::PushSelectedEntitiesToReparentQueue(Entity* parent)
    {
      // Change the selected files hierarchy
      EntityRawPtrArray& selected = m_draggingEntities;

      if (parent->GetType() == EntityType::Entity_Prefab)
      {
        return;
      }
      for (int i = 0; i < selected.size(); i++)
      {
        bool sameParent = selected[i]->GetIdVal() != parent->GetIdVal();
        bool isPrefab = selected[i]->GetType() == EntityType::Entity_Prefab;

        if (sameParent && (!Prefab::GetPrefabRoot(selected[i]) || isPrefab))
        {
          g_reparentQueue.push(selected[i]->GetIdVal());
        }
      }
      g_parent = parent->GetIdVal();
    }

    void OutlinerWindow::SetItemState(Entity* e)
    {
      EditorScenePtr currScene = g_app->GetCurrentScene();
      bool itemHovered = ImGui::IsItemHovered();
      m_anyEntityHovered |= itemHovered;

      if (itemHovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
      {
        bool ctrlDown   = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
        bool shiftDown  = ImGui::IsKeyDown(ImGuiKey_LeftShift);
        bool isSelected = currScene->IsSelected(e->GetIdVal());

        if (!shiftDown && !ctrlDown)
        {
          // this means not multiselecting so select only this
          currScene->ClearSelection();
          currScene->AddToSelection(e->GetIdVal(), true);
        }
        else if (ctrlDown && isSelected)
        {
          currScene->RemoveFromSelection(e->GetIdVal());
        }
        else if (shiftDown && m_lastClickedEntity != nullptr)
        {
          SelectEntitiesBetweenNodes(currScene.get(), m_lastClickedEntity, e);
        }
        else
        {
          currScene->AddToSelection(e->GetIdVal(), true);
          g_app->GetPropInspector()->m_activeView = PropInspector::ViewType::Entity;
        }
        m_lastClickedEntity = e;
      }
      
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
      {
        ImGui::SetDragDropPayload("HierarcyChange", nullptr, 0);
        ImGui::Text("Drop on the new parent.");
      
        m_draggingEntities.clear();

        if (!currScene->IsSelected(e->GetIdVal()))
        {
          m_draggingEntities.push_back(e);
        }
        else
        {
          currScene->GetSelectedEntities(m_draggingEntities);
        }

        ImGui::EndDragDropSource();
      }

      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("HierarcyChange"))
        {
          SortDraggedEntitiesByNodeIndex();
          PushSelectedEntitiesToReparentQueue(e);
          m_draggingEntities.clear();
        }
        ImGui::EndDragDropTarget();
      }
    }

    void OutlinerWindow::HandleSearch(const EntityRawPtrArray& ntties,
                                      const EntityRawPtrArray& roots)
    {
      if (ImGui::IsKeyPressed(ImGuiKey_Enter, false) && IsActive())
      {
        m_stringSearchMode = true;
      }

      m_stringSearchMode = m_stringSearchMode && !m_searchString.empty();

      // Clear shown entity map
      for (Entity* ntt : ntties)
      {
        if (m_searchString.empty())
        {
          m_shownEntities.insert(ntt);
        }
      }
      if (m_searchString.size() > 0)
      {
        // Find which entities should be shown
        for (Entity* e : roots)
        {
          FindShownEntities(e, m_searchString);
        }
      }
    }

    bool OutlinerWindow::FindShownEntities(Entity* e, const String& str)
    {
      bool self     = Utf8CaseInsensitiveSearch(e->GetNameVal(), str);

      bool children = false;
      if (e->GetType() != EntityType::Entity_Prefab)
      {
        for (Node* n : e->m_node->m_children)
        {
          Entity* childNtt = n->m_entity;
          if (childNtt != nullptr)
          {
            bool child = FindShownEntities(childNtt, str);
            children   = child || children;
          }
        }
      }
      bool isShown = self || children;
      if (isShown)
      {
        m_shownEntities.insert(e);
      }
      return isShown;
    }

    void OutlinerWindow::TryReorderEntites(float treeStartY)
    {
      if (m_indexToEntity.size() == 0 || m_draggingEntities.size() == 0)
      {
        return;
      }
      
      const float lineHeight = GetLineHeight();
      const float mouseY = ImGui::GetMousePos().y;
      int selectedIndex = (int)floorf((mouseY - treeStartY) / lineHeight);
      // min index is 0
      selectedIndex = glm::clamp(selectedIndex, 0, int(m_indexToEntity.size()) - 1);
      
      Entity* droppedBelowNtt = m_indexToEntity[selectedIndex];
      
      if (contains(m_draggingEntities, droppedBelowNtt))
      {
        GetLogger()->WriteConsole(LogType::Memo, 
                                 "cannot reorder if you drag below a selected entity");
        return;
      }

      const auto isRootFn = [](Entity* entity) -> bool
      { 
         return entity->m_node->m_parent == nullptr; 
      };
      
      bool allRoots = true;
      bool allSameParent = true;
      Node* firstParent  = m_draggingEntities[0]->m_node->m_parent;

      for (int i = 0; i < m_draggingEntities.size(); ++i)
      {
        allRoots &= isRootFn(m_draggingEntities[i]);
        allSameParent &= m_draggingEntities[i]->m_node->m_parent == firstParent;
      }
      
      if (!allSameParent)
      {
        GetLogger()->WriteConsole(LogType::Memo, "all selected entities should have same parent");
        return;
      }

      SortDraggedEntitiesByNodeIndex();

      Node* droppedParent = droppedBelowNtt->m_node->m_parent;
      bool droppedAboveFirstChild = false;

      // detect if we dropped above first children
      if (selectedIndex + 1 < (int)m_indexToEntity.size())
      {
        Node* nextNode = m_indexToEntity[selectedIndex + 1]->m_node;
        NodePtrArray& droppedBelowChildren = droppedBelowNtt->m_node->m_children;
        
        if (contains(droppedBelowChildren, nextNode))
        {
          droppedParent = droppedBelowNtt->m_node;
          droppedAboveFirstChild = true;
        }
      }

      if (allRoots && isRootFn(droppedBelowNtt) && !droppedAboveFirstChild)
      {
        // remove all dropped entites 
        EditorScenePtr scene = g_app->GetCurrentScene();
        scene->RemoveEntity(m_draggingEntities);
        EntityRawPtrArray& sceneEntities = scene->AccessEntityArray();
        // find index of dropped entity and
        // insert all dropped entities below dropped entity
        sceneEntities.insert(
          std::find(sceneEntities.begin(), sceneEntities.end(), droppedBelowNtt) + 1,
          m_draggingEntities.begin(),
          m_draggingEntities.end());
      }
      else
      {
        for (int i = 0; i < m_draggingEntities.size(); i++)
        {
          m_draggingEntities[i]->m_node->OrphanSelf(true);
        }
        
        if (droppedParent != nullptr)
        {
          std::vector<Node*>& childs = droppedParent->m_children;

          int index = droppedAboveFirstChild ? -1
                          : std::find(childs.begin(), childs.end(),
                            droppedBelowNtt->m_node) - childs.begin();

          for (int i = 0; i < m_draggingEntities.size(); ++i)
          {
            Node* node = m_draggingEntities[i]->m_node;
            droppedParent->InsertChild(node, index + i + 1, true);
          }
        }
      }
      m_draggingEntities.clear();
    }

    void OutlinerWindow::Show()
    {
      EditorScenePtr currScene = g_app->GetCurrentScene();

      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        odd = 0;
        m_anyEntityHovered = false;
        HandleStates();
        ShowSearchBar(m_searchString);
        ImGui::BeginChild("##Outliner Nodes");
        ImGuiTreeNodeFlags flag =
            g_treeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen;

        float treeStartY = ImGui::GetCursorScreenPos().y;

        if (DrawRootHeader("Scene", 0, flag, UI::m_collectionIcon))
        {
          const EntityRawPtrArray& ntties = currScene->GetEntities();
          m_roots.clear();
          treeStartY = ImGui::GetCursorScreenPos().y;

          GetRootEntities(ntties, m_roots);
          HandleSearch(ntties, m_roots);
          m_indexToEntity.clear();

          for (size_t i = 0; i < m_roots.size(); ++i)
          {
            ShowNode(m_roots[i], 0);
          }

          ImGui::TreePop();
        }

        // if we click an empty space in this window. selection list will cleared
        bool multiSelecting = ImGui::IsKeyDown(ImGuiKey_LeftShift) || 
                                ImGui::IsKeyDown(ImGuiKey_LeftCtrl);

        bool windowSpaceHovered = !m_anyEntityHovered && ImGui::IsWindowHovered();
        bool mouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

        if (windowSpaceHovered && !multiSelecting && mouseReleased)
        {
          TryReorderEntites(treeStartY);
        
          for (int i = 0; i < m_draggingEntities.size(); ++i)
          {
            Entity* entity = m_draggingEntities[i];
            entity->m_node->OrphanSelf(true);
          }
          m_draggingEntities.clear();
          currScene->ClearSelection();
        }

        ImGui::EndChild();
      }

      // Update hierarchy if there is a change.
      while (!g_reparentQueue.empty())
      {
        Entity* child = currScene->GetEntity(g_reparentQueue.top());
        child->m_node->OrphanSelf(true);

        if (g_parent != NULL_HANDLE)
        {
          Entity* parent = currScene->GetEntity(g_parent);
          parent->m_node->AddChild(child->m_node, true);
        }
        g_reparentQueue.pop();
      }

      g_parent = NULL_HANDLE;

      ImGui::End();
    }

    Window::Type OutlinerWindow::GetType() const
    {
      return Window::Type::Outliner;
    }

    void OutlinerWindow::DispatchSignals() const { ModShortCutSignals(); }

    void OutlinerWindow::Focus(Entity* ntt)
    {
      m_nttFocusPath.push_back(ntt);
      GetParents(ntt, m_nttFocusPath);
    }

    bool OutlinerWindow::DrawRootHeader(const String& rootName,
                                        uint id,
                                        ImGuiTreeNodeFlags flags,
                                        TexturePtr icon)
    {
      const String sId = "##" + std::to_string(id);
      bool isOpen      = ImGui::TreeNodeEx(sId.c_str(), flags);

      // Orphan in this case.
      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("HierarcyChange"))
        {
          EntityRawPtrArray selected;
          EditorScenePtr currScene = g_app->GetCurrentScene();
          currScene->GetSelectedEntities(selected);

          for (int i = 0; i < selected.size(); i++)
          {
            if (selected[i]->GetIdVal() != NULL_HANDLE &&
                (!Prefab::GetPrefabRoot(selected[i]) ||
                 selected[i]->GetType() == EntityType::Entity_Prefab))
            {
              g_reparentQueue.push(selected[i]->GetIdVal());
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

    void OutlinerWindow::ShowSearchBar(String& searchString)
    {
      ImGui::BeginTable("##Search", 2, ImGuiTableFlags_SizingFixedFit);
      ImGui::TableSetupColumn("##SearchBar",
                              ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("##ToggleCaseButton");

      ImGui::TableNextColumn();

      ImGui::PushItemWidth(-1);
      ImGui::InputTextWithHint(" SearchString", "Search", &searchString);
      ImGui::PopItemWidth();

      ImGui::TableNextColumn();

      m_searchCaseSens =
          UI::ToggleButton("Aa", Vec2(24.0f, 24.f), m_searchCaseSens);

      UI::HelpMarker(TKLoc, "Case Sensitivity");

      ImGui::EndTable();
    }

    // customized version of this: https://github.com/ocornut/imgui/issues/2668
    // draws a rectangle on tree node, for even odd pattern
    void OutlinerWindow::DrawRowBackground(int depth)
    {
      depth = glm::min(depth, 7); // 7 array max size 
      // offsets on starting point of the rectangle
      float depths[] = // last two of the offsets are not adjusted well enough 
          {18.0f, 30.0f, 51.0f, 71.0f, 96.0f, 115.0f, 140.0f, 155.0f};
      
      ImRect workRect      = ImGui::GetCurrentWindow()->WorkRect;
      float x1             = workRect.Min.x + depths[depth];
      float x2             = workRect.Max.x;
      float item_spacing_y = ImGui::GetStyle().ItemSpacing.y;
      float item_offset_y  = -item_spacing_y * 0.5f;
      float line_height    = ImGui::GetTextLineHeight() + item_spacing_y;
      float y0 = ImGui::GetCursorScreenPos().y + (float) item_offset_y;

      ImDrawList* draw_list  = ImGui::GetWindowDrawList();
      ImGuiStyle& style      = ImGui::GetStyle();
      ImVec4 v4Color         = style.Colors[ImGuiCol_TabHovered];
      v4Color.x             *= 0.62f;
      v4Color.y             *= 0.62f;
      v4Color.z             *= 0.62f;
      // if odd black otherwise given color
      ImU32 col = ImGui::ColorConvertFloat4ToU32(v4Color) * (odd++ & 1);

      if (col == 0)
      {
        return;
      }
      float y1 = y0 + line_height;
      draw_list->AddRectFilled(ImVec2(x1, y0), ImVec2(x2, y1), col);
    }

    bool OutlinerWindow::DrawHeader(Entity* ntt,
                                    ImGuiTreeNodeFlags flags,
                                    int depth)
    {
      if (ntt->GetNameVal().find(m_searchString) != String::npos)
      {
        m_stringSearchMode = false;
      }

      bool focusToItem  = false;
      bool nextItemOpen = false;
      if (!m_nttFocusPath.empty())
      {
        int focusIndx = IndexOf(ntt, m_nttFocusPath);
        if (focusIndx != -1)
        {
          nextItemOpen = true;
        }

        focusToItem = focusIndx == 0;
      }

      if (nextItemOpen || m_stringSearchMode)
      {
        ImGui::SetNextItemOpen(true);
      }
      // bright and dark color pattern for nodes. (even odd)
      DrawRowBackground(depth);

      const String sId = "##" + std::to_string(ntt->GetIdVal());
      // blue highlight for tree node on mouse hover
      ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                            ImVec4(0.3f, 0.4f, 0.7f, 0.5f)); 
      ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.4f, 0.5f, 0.8f, 1.0f)); 
      bool isOpen      = ImGui::TreeNodeEx(sId.c_str(), flags);
      ImGui::PopStyleColor(2); 

      if (ImGui::BeginPopupContextItem())
      {
        if (ImGui::MenuItem("SaveAsPrefab"))
        {
          GetSceneManager()->GetCurrentScene()->SavePrefab(ntt);
          for (FolderWindow* browser : g_app->GetAssetBrowsers())
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

        if (ImGui::MenuItem("Delete"))
        {
          ModManager::GetInstance()->DispatchSignal(BaseMod::m_delete);
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

      // show name, open and slash eye, lock and unlock.
      UI::ShowEntityTreeNodeContent(ntt);

      return isOpen;
    }

  } // namespace Editor
} // namespace ToolKit
  