#include "OutlinerWindow.h"

#include "App.h"
#include "FolderWindow.h"
#include "Global.h"
#include "IconsFontAwesome.h"
#include "Mod.h"
#include "UI.h"
#include "Util.h"
#include "TopBar.h"
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
      rectMin.x = cursorPos.x - 11.0f;
      rectMin.y += halfHeight;

      float bottom = rectMin.y + (numNodes * line_height);
      bottom -= (halfHeight + 1.0f); // move up a little

      ImDrawList* drawList = ImGui::GetWindowDrawList();
      const ImColor color = ImGui::GetColorU32(ImGuiCol_Text);
      drawList->AddLine(rectMin, ImVec2(rectMin.x, bottom), color);
      // a little bulge at the end of the line
      drawList->AddLine(ImVec2(rectMin.x, bottom),
                        ImVec2(rectMin.x + 5.0f, bottom),
                        color);
    }

    // returns total drawed nodes
    int OutlinerWindow::ShowNode(Entity* e, int depth)
    {
      // if searching mode is on and entity is not shown return.
      if (m_stringSearchMode == true && m_shownEntities.count(e) == 0)
      {
        return 0;
      }

      ImGuiTreeNodeFlags nodeFlags = g_treeNodeFlags;
      EditorScenePtr currScene = g_app->GetCurrentScene();
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
      std::sort(m_draggingEntities.begin(),
                m_draggingEntities.end(),
                [this](Entity* a, Entity* b) -> bool {
                  return FindIndex(m_indexToEntity, a) <
                         FindIndex(m_indexToEntity, b);
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
        for (; i < m_roots.size() && numFound != 2; ++i)
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

      if (itemHovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
      {
        bool ctrlDown = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
        bool shiftDown = ImGui::IsKeyDown(ImGuiKey_LeftShift);
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
    
    bool OutlinerWindow::FindShownEntities(Entity* e, const String& str)
    {
      bool self = Utf8CaseInsensitiveSearch(e->GetNameVal(), str);

      bool children = false;
      if (e->GetType() != EntityType::Entity_Prefab)
      {
        for (Node* n : e->m_node->m_children)
        {
          Entity* childNtt = n->m_entity;
          if (childNtt != nullptr)
          {
            bool child = FindShownEntities(childNtt, str);
            children = child || children;
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

    //   entity_123
    //   ---------- <- returns true if you indicate here
    //   entity_321
    bool OutlinerWindow::IndicatingInBetweenNodes()
    {
      return !m_anyEntityHovered && ImGui::IsWindowHovered();
    }

    // indicates that dropped on top of all entities
    static const int DroppedOnTopOfEntities = -1;

    // if you are indicating between two nodes this will 
    // return the index of upper entity, 
    // if you are indicating on top of all entities this will return DroppedOnTopOfEntities
    int OutlinerWindow::GetMouseHoveredNodeIndex(float treeStartY) const
    {
      const float lineHeight = GetLineHeight();
      const float mouseY = ImGui::GetMousePos().y;

      // is dropped to top of the first entity?
      if (fabsf(mouseY - treeStartY) < lineHeight * 0.5)
      {
        return DroppedOnTopOfEntities;
      }

      int selectedIndex = (int)floorf((mouseY - treeStartY) / lineHeight);
      int maxIdx        = glm::max(0, int(m_indexToEntity.size()) - 1);
      return glm::clamp(selectedIndex, 0, maxIdx);
    }

    void OrphanAll(const EntityRawPtrArray& movedEntities)
    {
      for (Entity* e : movedEntities)
      {
        e->m_node->OrphanSelf(true);
      }
    }

    bool OutlinerWindow::IsInsertingAtTheEndOfEntities()
    {
      return m_insertSelectedIndex == TK_INT_MAX;
    }

    // the idea behind is: 
    // * detect if we can reorder or not. if so:
    //     orphan all movedEntities
    //     remove all movedEntities from the scene (entities array)
    // * if all of the moved entities are root entities and the entity we dropped below is also root
    //   insert all entities below dropped entity (because we removed them)
    // * else if the entity we dropped bellow is not root
    //   this means we dropped in childs list. detect the child index
    //   insert all moved entities to parents children (between the index we detect)
    bool OutlinerWindow::TryReorderEntites(const EntityRawPtrArray& movedEntities)
    {
      // check number of visible entities in outliner is zero or inserted entities.size is zero
      if (m_indexToEntity.size() == 0 || movedEntities.size() == 0)
      {
        return false;
      }
      
      int selectedIndex = m_insertSelectedIndex;
      EditorScenePtr scene = g_app->GetCurrentScene();
      EntityRawPtrArray& entities = scene->AccessEntityArray();

      SortDraggedEntitiesByNodeIndex();
      // is dropped to on top of the first entity?  
      if (selectedIndex == DroppedOnTopOfEntities)
      {
        OrphanAll(movedEntities);
        scene->RemoveEntity(movedEntities);
        entities.insert(entities.begin(), movedEntities.begin(), movedEntities.end());
        return true;
      }
      
      selectedIndex = glm::clamp(selectedIndex, 0, int(m_indexToEntity.size())-1);
      Entity* droppedBelowNtt = m_indexToEntity[selectedIndex];

      if (contains(movedEntities, droppedBelowNtt))
      {
        GetLogger()->WriteConsole(LogType::Memo, 
                                 "cannot reorder if you drag below a selected entity");
        return false;
      }
 
      bool allSameParent = true;
      Node* firstParent  = movedEntities[0]->m_node->m_parent;

      for (int i = 0; i < movedEntities.size(); ++i)      
      {
        allSameParent &= movedEntities[i]->m_node->m_parent == firstParent;
      }
      
      if (!allSameParent)
      {
        GetLogger()->WriteConsole(LogType::Memo, "all selected entities should have same parent");
        return false;
      }

      Node* droppedParent = droppedBelowNtt->m_node->m_parent;
      int childIndex = 0;
      bool droppedAboveFirstChild = false;

      // detect if we dropped above first children. childIndex will stay 0
      //  EntityParent
      //  ------------- <- did we drop here?
      //    EntityChild0
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
      
      const auto isRootFn = [](Entity* entity) -> bool
      { 
         return entity->m_node->m_parent == nullptr; 
      };

      OrphanAll(movedEntities);
      // the object that we dropped below is root ?
      if (isRootFn(droppedBelowNtt) && !droppedAboveFirstChild)
      {
        // remove all dropped entites 
        scene->RemoveEntity(movedEntities);
        // find index of dropped entity and
        // insert all dropped entities below dropped entity 
        auto find = std::find(entities.cbegin(), entities.cend(), droppedBelowNtt);
        entities.insert(find + 1, movedEntities.begin(), movedEntities.end());
      }
      else if (droppedParent != nullptr) // did we drop in child list?
      {
        if (!droppedAboveFirstChild)
        {
          NodePtrArray& childs = droppedParent->m_children;
          childIndex = FindIndex(childs, droppedBelowNtt->m_node) + 1;
        }

        for (int i = 0; i < movedEntities.size(); ++i)
        {
          Node* node = movedEntities[i]->m_node;
          droppedParent->InsertChild(node, childIndex + i, true);
        }
      }
      // reset to default insert index (end of the list)
      m_insertSelectedIndex = TK_INT_MAX;
      return true;
    }

    void OutlinerWindow::Show()
    {
      EditorScenePtr currScene = g_app->GetCurrentScene();

      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        odd = 0;
        m_anyEntityHovered = false;

        const EntityRawPtrArray& ntties = currScene->GetEntities();
        m_roots.clear();
        m_indexToEntity.clear();

        std::copy_if(ntties.cbegin(),
                     ntties.cend(),
                     std::back_inserter(m_roots),
                     [](const Entity* e)
                     { return e->m_node->m_parent == nullptr; });
        
        HandleStates();
        ShowSearchBar(m_searchString);

        ImGui::BeginChild("##Outliner Nodes");
        ImGuiTreeNodeFlags flag =
            g_treeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen;

        m_treeStartY = ImGui::GetCursorScreenPos().y;

        if (DrawRootHeader("Scene", 0, flag, UI::m_collectionIcon))
        {
          m_treeStartY = ImGui::GetCursorScreenPos().y;

          for (size_t i = 0; i < m_roots.size(); ++i)
          {
            ShowNode(m_roots[i], 0);
          }

          ImGui::TreePop();
        }

        // if we click an empty space in this window. selection list will cleared
        bool leftMouseReleased  = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
        bool rightMouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Right);
        bool dragging = m_draggingEntities.size() > 0ull;

        bool multiSelecting = ImGui::IsKeyDown(ImGuiKey_LeftShift) ||
                              ImGui::IsKeyDown(ImGuiKey_LeftCtrl);

        if (!multiSelecting && IndicatingInBetweenNodes()) 
        {
          if (leftMouseReleased && dragging)
          {
            // we are using this member variable in try insert entities function
            // (order is important. dont move this below if block)
            m_insertSelectedIndex = GetMouseHoveredNodeIndex(m_treeStartY); 
            
            if (!TryReorderEntites(m_draggingEntities))
            {
              // if reordering is not possible orphan all dragged entities
              // because we clicked to an empty space.
              OrphanAll(m_draggingEntities);
            }
            m_draggingEntities.clear();
            currScene->ClearSelection();
          }
          // right click in between entities.
          if (rightMouseReleased)
          {
            ImGui::OpenPopup("##Create");
            // fill required argument for reordering.
            m_insertSelectedIndex = GetMouseHoveredNodeIndex(m_treeStartY); 
          }
        }
        
        if (leftMouseReleased) 
        {
          m_draggingEntities.clear();
        }

        // show drag drop tooltip
        if (dragging) 
        {
          if (m_anyEntityHovered && m_indexToEntity.size() > 0ull)
          {
            int index = glm::clamp(GetMouseHoveredNodeIndex(m_treeStartY),
                                   0,
                                   int(m_indexToEntity.size())-1);
            Entity* hoveredEntity = m_indexToEntity[index];
            ImGui::SetTooltip(contains(m_draggingEntities, hoveredEntity)
                                  ? "Drag Drop for set as child or Reorder"
                                  : "Set As Child");
          }
          else
          {
            ImGui::SetTooltip("Reorder Entities");
          }
        }

        if (ImGui::BeginPopup("##Create"))
        {
          // this function will call TryReorderEntities 
          // (if we click one of the creation buttons)
          OverlayTopBar::ShowAddMenuPopup();
          ImGui::EndPopup();
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
      
      // algorithm will search only if we type. 
      // if string is empty search mode is of
      if (ImGui::InputTextWithHint(" SearchString", "Search", &searchString))
      {
        m_stringSearchMode = searchString.size() > 0ull;
        bool searchStrChanged = m_searchStringSize != (int)searchString.size();
        m_searchStringSize = (int)searchString.size();

        if (searchStrChanged && m_stringSearchMode)
        {
          m_shownEntities.clear();
          // Find which entities should be shown
          for (Entity* e : m_roots)
          {
            FindShownEntities(e, m_searchString);
          }
        }
      }

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

      m_anyEntityHovered |= ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);

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
  