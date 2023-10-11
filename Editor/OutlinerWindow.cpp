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

#include "OutlinerWindow.h"

#include "App.h"
#include "ImGui/imgui_internal.h"
#include "Mod.h"
#include "Prefab.h"
#include "TopBar.h"

#include <MathUtil.h>

#include <stack>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

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
      float line_height     = GetLineHeight();
      float halfHeight      = line_height * 0.5f;

      ImVec2 cursorPos      = ImGui::GetCursorScreenPos();
      // -11 align line with arrow
      rectMin.x             = cursorPos.x - 11.0f;
      rectMin.y            += halfHeight;

      float bottom          = rectMin.y + (numNodes * line_height);
      bottom               -= (halfHeight + 1.0f); // move up a little

      ImDrawList* drawList  = ImGui::GetWindowDrawList();
      const ImColor color   = ImGui::GetColorU32(ImGuiCol_Text);
      drawList->AddLine(rectMin, ImVec2(rectMin.x, bottom), color);
      // a little bulge at the end of the line
      drawList->AddLine(ImVec2(rectMin.x, bottom), ImVec2(rectMin.x + 5.0f, bottom), color);
    }

    // returns total drawed nodes
    int OutlinerWindow::ShowNode(EntityPtr ntt, int depth)
    {
      // if searching mode is on and entity or its parents are not shown return.
      if (m_stringSearchMode == true)
      {
        bool parentsOrSelfOpen = false;
        Node* parent           = ntt->m_node;

        while (parent != nullptr)
        {
          parentsOrSelfOpen |= m_shownEntities.count(parent->m_entity) > 0;
          parent             = parent->m_parent;
        }

        if (!parentsOrSelfOpen)
        {
          return 0;
        }
      }

      ImGuiTreeNodeFlags nodeFlags = g_treeNodeFlags;
      EditorScenePtr currScene     = g_app->GetCurrentScene();
      if (currScene->IsSelected(ntt->GetIdVal()))
      {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
      }

      int numNodes = 1; // 1 itself and we will sum childs
      m_indexToEntity.push_back(ntt);

      if (ntt->m_node->m_children.empty() || ntt->IsA<Prefab>())
      {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        DrawHeader(ntt, nodeFlags, depth);
      }
      else
      {
        if (DrawHeader(ntt, nodeFlags, depth))
        {
          ImVec2 rectMin = ImGui::GetItemRectMin();

          for (Node* n : ntt->m_node->m_children)
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
    // we need to sort dragged entities in order to preserve order when we move
    // the entities
    void OutlinerWindow::SortDraggedEntitiesByNodeIndex()
    {
      std::sort(m_draggingEntities.begin(),
                m_draggingEntities.end(),
                [this](EntityPtr a, EntityPtr b) -> bool
                { return FindIndex(m_indexToEntity, a) < FindIndex(m_indexToEntity, b); });
    }

    void OutlinerWindow::SelectEntitiesBetweenNodes(EditorScenePtr scene, EntityPtr a, EntityPtr b)
    {
      if (a == b)
      {
        return;
      }

      if (a->m_node->m_parent != b->m_node->m_parent)
      {
        GetLogger()->WriteConsole(LogType::Warning, "selected entities should have same parent!");
        return;
      }

      size_t i     = 0ull;
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

      NodeRawPtrArray& children = a->m_node->m_parent->m_children;
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

    void OutlinerWindow::PushSelectedEntitiesToReparentQueue(EntityPtr parent)
    {
      // Change the selected files hierarchy
      EntityPtrArray& selected = m_draggingEntities;

      if (parent->IsA<Prefab>())
      {
        return;
      }
      for (int i = 0; i < selected.size(); i++)
      {
        bool sameParent = selected[i]->GetIdVal() != parent->GetIdVal();
        bool isPrefab   = selected[i]->IsA<Prefab>();

        if (sameParent && (!Prefab::GetPrefabRoot(selected[i]) || isPrefab))
        {
          g_reparentQueue.push(selected[i]->GetIdVal());
        }
      }
      g_parent = parent->GetIdVal();
    }

    void OutlinerWindow::SetItemState(EntityPtr ntt)
    {
      EditorScenePtr currScene = g_app->GetCurrentScene();
      bool itemHovered         = ImGui::IsItemHovered();

      if (itemHovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
      {
        bool ctrlDown   = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
        bool shiftDown  = ImGui::IsKeyDown(ImGuiKey_LeftShift);
        bool isSelected = currScene->IsSelected(ntt->GetIdVal());

        if (!shiftDown && !ctrlDown)
        {
          // this means not multiselecting so select only this
          currScene->ClearSelection();
          currScene->AddToSelection(ntt->GetIdVal(), true);
        }
        else if (ctrlDown && isSelected)
        {
          currScene->RemoveFromSelection(ntt->GetIdVal());
        }
        else if (shiftDown && m_lastClickedEntity != nullptr)
        {
          SelectEntitiesBetweenNodes(currScene, m_lastClickedEntity, ntt);
        }
        else
        {
          currScene->AddToSelection(ntt->GetIdVal(), true);
          g_app->GetPropInspector()->m_activeView = ViewType::Entity;
        }
        m_lastClickedEntity = ntt;
      }

      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
      {
        ImGui::SetDragDropPayload("HierarcyChange", nullptr, 0);
        m_draggingEntities.clear();

        if (!currScene->IsSelected(ntt->GetIdVal()))
        {
          m_draggingEntities.push_back(ntt);
        }
        else
        {
          currScene->GetSelectedEntities(m_draggingEntities);
        }

        ImGui::EndDragDropSource();
      }

      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HierarcyChange"))
        {
          SortDraggedEntitiesByNodeIndex();
          PushSelectedEntitiesToReparentQueue(ntt);
          m_draggingEntities.clear();
        }
        ImGui::EndDragDropTarget();
      }
    }

    bool OutlinerWindow::FindShownEntities(EntityPtr ntt, const String& str)
    {
      bool self     = Utf8CaseInsensitiveSearch(ntt->GetNameVal(), str);
      bool children = false;

      if (!ntt->IsA<Prefab>())
      {
        for (Node* n : ntt->m_node->m_children)
        {
          EntityPtr childNtt = n->m_entity;
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
        m_shownEntities.insert(ntt);
      }

      return isShown;
    }

    //   entity_123
    //   ---------- <- returns true if you indicate here
    //   entity_321
    bool OutlinerWindow::IndicatingInBetweenNodes() { return !m_anyEntityHovered && ImGui::IsWindowHovered(); }

    // indicates that dropped on top of all entities
    const int DroppedOnTopOfEntities  = -1;
    const int DroppedBelowAllEntities = TK_INT_MAX;
    // max -1 because if we set m_insertSelectedIndex
    // we want to drop below all entities ( clamp function will cause that)
    const int DropIsNotPossible       = TK_INT_MAX - 1;

    // if you are indicating between two nodes this will
    // return the index of upper entity,
    // if you are indicating on top of all entities this will return
    // DroppedOnTopOfEntities
    int OutlinerWindow::GetMouseHoveredNodeIndex(float treeStartY) const
    {
      const float lineHeight = GetLineHeight();
      const Vec2 mousePos    = ImGui::GetMousePos();
      const Vec2 windowPos   = ImGui::GetWindowPos();
      const Vec2 windowSize  = ImGui::GetWindowSize();

      // is dropped to top of the first entity?
      if (glm::abs(mousePos.y - treeStartY) < lineHeight * 0.5f)
      {
        return DroppedOnTopOfEntities;
      }

      const float halfLineHeight = lineHeight * 0.5f;
      Vec2 bottomRectMin         = windowPos + Vec2(0.0f, windowSize.y - halfLineHeight);
      Vec2 bottomRectMax         = windowPos + Vec2(windowSize.x, windowSize.y + halfLineHeight);

      // is dropped below all entities?
      // a one line height rect that is bottom of the outliner window to check if we drop below all entities
      if (RectPointIntersection(bottomRectMin, bottomRectMax, mousePos))
      {
        return DroppedBelowAllEntities;
      }

      // order of this if block important,
      // we should call this after two if's above
      if (!ImGui::IsWindowHovered())
      {
        return DropIsNotPossible;
      }

      int selectedIndex = (int) glm::floor((mousePos.y - treeStartY) / lineHeight);
      int maxIdx        = glm::max(0, int(m_indexToEntity.size()) - 1);
      return glm::clamp(selectedIndex, 0, maxIdx);
    }

    void OrphanAll(const EntityPtrArray& movedEntities)
    {
      for (EntityPtr ntt : movedEntities)
      {
        ntt->m_node->OrphanSelf(true);
      }
    }

    bool OutlinerWindow::IsInsertingAtTheEndOfEntities() { return m_insertSelectedIndex == TK_INT_MAX; }

    // the idea behind is:
    // * detect if we can reorder or not. if so:
    //     orphan all movedEntities
    //     remove all movedEntities from the scene (entities array)
    // * if all of the moved entities are root entities and the entity we
    // dropped below is also root
    //   insert all entities below dropped entity (because we removed them)
    // * else if the entity we dropped bellow is not root
    //   this means we dropped in childs list. detect the child index
    //   insert all moved entities to parents children (between the index we
    //   detect)
    bool OutlinerWindow::TryReorderEntites(const EntityPtrArray& movedEntities)
    {
      // todo: reordering is not possible currently!!!
      return false;
      // check number of visible entities in outliner is zero or inserted
      // entities.size is zero
      if (m_indexToEntity.size() == 0 || movedEntities.size() == 0)
      {
        return false;
      }

      int selectedIndex        = m_insertSelectedIndex;
      EditorScenePtr scene     = g_app->GetCurrentScene();
      EntityPtrArray& entities = scene->AccessEntityArray();

      SortDraggedEntitiesByNodeIndex();
      // is dropped to on top of the first entity?
      if (selectedIndex == DroppedOnTopOfEntities)
      {
        OrphanAll(movedEntities);
        scene->RemoveEntity(movedEntities);
        entities.insert(entities.begin(), movedEntities.begin(), movedEntities.end());
        return true;
      }

      if (selectedIndex == DroppedBelowAllEntities)
      {
        OrphanAll(movedEntities);
        scene->RemoveEntity(movedEntities);
        entities.insert(entities.end(), movedEntities.begin(), movedEntities.end());
        return true;
      }

      selectedIndex             = glm::clamp(selectedIndex, 0, int(m_indexToEntity.size()) - 1);
      EntityPtr droppedBelowNtt = m_indexToEntity[selectedIndex];

      if (contains(movedEntities, droppedBelowNtt))
      {
        GetLogger()->WriteConsole(LogType::Memo, "cannot reorder if you drag below a selected entity");
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

      Node* droppedParent         = droppedBelowNtt->m_node->m_parent;
      int childIndex              = 0;
      bool droppedAboveFirstChild = false;

      // detect if we dropped above first children. childIndex will stay 0
      //  EntityParent
      //  ------------- <- did we drop here?
      //    EntityChild0
      if (selectedIndex + 1 < (int) m_indexToEntity.size())
      {
        Node* nextNode                        = m_indexToEntity[selectedIndex + 1]->m_node;
        NodeRawPtrArray& droppedBelowChildren = droppedBelowNtt->m_node->m_children;

        if (contains(droppedBelowChildren, nextNode))
        {
          droppedParent          = droppedBelowNtt->m_node;
          droppedAboveFirstChild = true;
        }
      }

      const auto isRootFn = [](EntityPtr entity) -> bool { return entity->m_node->m_parent == nullptr; };

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
          NodeRawPtrArray& childs = droppedParent->m_children;
          childIndex              = FindIndex(childs, droppedBelowNtt->m_node) + 1;
        }

        for (int i = 0; i < movedEntities.size(); ++i)
        {
          Node* node = movedEntities[i]->m_node;
          droppedParent->InsertChild(node, childIndex + i, true);
        }
      }
      // reset to default insert index (end of the list)
      m_insertSelectedIndex = DroppedBelowAllEntities;
      return true;
    }

    void OutlinerWindow::Show()
    {
      EditorScenePtr currScene = g_app->GetCurrentScene();

      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        odd                          = 0;
        m_anyEntityHovered           = false;

        const EntityPtrArray& ntties = currScene->GetEntities();
        m_roots.clear();
        m_indexToEntity.clear();

        // get root entities
        std::copy_if(ntties.cbegin(),
                     ntties.cend(),
                     std::back_inserter(m_roots),
                     [](const EntityPtr e) { return e->m_node->m_parent == nullptr; });

        HandleStates();
        ShowSearchBar(m_searchString);

        ImGui::BeginChild("##Outliner Nodes");
        ImGuiTreeNodeFlags flag = g_treeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen;

        m_treeStartY            = ImGui::GetCursorScreenPos().y;

        if (DrawRootHeader("Scene", 0, flag, UI::m_collectionIcon))
        {
          m_treeStartY = ImGui::GetCursorScreenPos().y;

          for (size_t i = 0; i < m_roots.size(); ++i)
          {
            ShowNode(m_roots[i], 0);
          }

          ImGui::TreePop();
        }

        // if we click an empty space in this window. selection list will
        // cleared
        bool leftMouseReleased  = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
        bool rightMouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Right);
        bool dragging           = m_draggingEntities.size() > 0ull;

        bool multiSelecting     = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_LeftCtrl);

        if (!multiSelecting && !m_anyEntityHovered)
        {
          if (leftMouseReleased || rightMouseReleased)
          {
            // fill required argument for reordering.
            // we are using this member variable in TryReorderEntites function
            // (order is important. dont move this below next if block)
            m_insertSelectedIndex = GetMouseHoveredNodeIndex(m_treeStartY);
          }
          bool canInsert = m_insertSelectedIndex != DropIsNotPossible;

          if (leftMouseReleased && dragging && canInsert)
          {
            if (!TryReorderEntites(m_draggingEntities))
            {
              // if reordering is not possible orphan all dragged entities
              // because we clicked to an empty space.
              OrphanAll(m_draggingEntities);
            }
            currScene->ClearSelection();
            currScene->ValidateBillboard(m_draggingEntities);
            m_draggingEntities.clear();
          }

          // right click in between entities.
          if (rightMouseReleased && canInsert)
          {
            ImGui::OpenPopup("##Create");
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
            int index = glm::clamp(GetMouseHoveredNodeIndex(m_treeStartY), 0, int(m_indexToEntity.size()) - 1);
            EntityPtr hoveredEntity = m_indexToEntity[index];
            ImGui::SetTooltip(contains(m_draggingEntities, hoveredEntity) ? "Drag Drop for set as child or Reorder"
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
        EntityPtr child = currScene->GetEntity(g_reparentQueue.top());
        child->m_node->OrphanSelf(true);

        if (g_parent != NULL_HANDLE)
        {
          EntityPtr parent = currScene->GetEntity(g_parent);
          parent->m_node->AddChild(child->m_node, true);
        }
        g_reparentQueue.pop();
      }

      g_parent = NULL_HANDLE;

      ImGui::End();
    }

    Window::Type OutlinerWindow::GetType() const { return Window::Type::Outliner; }

    void OutlinerWindow::DispatchSignals() const { ModShortCutSignals(); }

    void OutlinerWindow::Focus(EntityPtr ntt)
    {
      m_nttFocusPath.push_back(ntt);
      GetParents(ntt, m_nttFocusPath);
    }

    void OutlinerWindow::ClearOutliner()
    {
      m_nttFocusPath.clear();
      m_shownEntities.clear();
      m_indexToEntity.clear();
      m_draggingEntities.clear();
      m_roots.clear();
      m_lastClickedEntity = nullptr;
      m_rootsParent       = nullptr;
    }

    bool OutlinerWindow::DrawRootHeader(const String& rootName, uint id, ImGuiTreeNodeFlags flags, TexturePtr icon)
    {
      const String sId = "##" + std::to_string(id);
      bool isOpen      = ImGui::TreeNodeEx(sId.c_str(), flags);

      // Orphan in this case.
      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HierarcyChange"))
        {
          EntityPtrArray selected;
          EditorScenePtr currScene = g_app->GetCurrentScene();
          currScene->GetSelectedEntities(selected);

          for (int i = 0; i < selected.size(); i++)
          {
            if (selected[i]->GetIdVal() != NULL_HANDLE &&
                (!Prefab::GetPrefabRoot(selected[i]) || selected[i]->IsA<Prefab>()))
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
      ImGui::TableSetupColumn("##SearchBar", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("##ToggleCaseButton");

      ImGui::TableNextColumn();

      ImGui::PushItemWidth(-1);

      // algorithm will search only if we type.
      // if string is empty search mode is of
      if (ImGui::InputTextWithHint(" SearchString", "Search", &searchString))
      {
        m_stringSearchMode    = searchString.size() > 0ull;
        bool searchStrChanged = m_searchStringSize != (int) searchString.size();
        m_searchStringSize    = (int) searchString.size();

        if (searchStrChanged && m_stringSearchMode)
        {
          m_shownEntities.clear();
          // Find which entities should be shown
          for (EntityPtr ntt : m_roots)
          {
            FindShownEntities(ntt, m_searchString);
          }
        }
      }
      m_stringSearchMode = searchString.size() > 0ull;

      ImGui::PopItemWidth();

      ImGui::TableNextColumn();

      m_searchCaseSens = UI::ToggleButton("Aa", Vec2(24.0f, 24.f), m_searchCaseSens);

      UI::HelpMarker(TKLoc, "Case Sensitivity");

      ImGui::EndTable();
    }

    // customized version of this: https://github.com/ocornut/imgui/issues/2668
    // draws a rectangle on tree node, for even odd pattern
    void OutlinerWindow::DrawRowBackground(int depth)
    {
      depth          = glm::min(depth, 7); // 7 array max size
      // offsets on starting point of the rectangle
      float depths[] = // last two of the offsets are not adjusted well enough
          {18.0f, 30.0f, 51.0f, 71.0f, 96.0f, 115.0f, 140.0f, 155.0f};

      ImRect workRect        = ImGui::GetCurrentWindow()->WorkRect;
      float x1               = workRect.Min.x + depths[depth];
      float x2               = workRect.Max.x;
      float item_spacing_y   = ImGui::GetStyle().ItemSpacing.y;
      float item_offset_y    = -item_spacing_y * 0.5f;
      float line_height      = ImGui::GetTextLineHeight() + item_spacing_y;
      float y0               = ImGui::GetCursorScreenPos().y + (float) item_offset_y;

      ImDrawList* draw_list  = ImGui::GetWindowDrawList();
      ImGuiStyle& style      = ImGui::GetStyle();
      ImVec4 v4Color         = style.Colors[ImGuiCol_TabHovered];
      v4Color.x             *= 0.62f;
      v4Color.y             *= 0.62f;
      v4Color.z             *= 0.62f;
      // if odd black otherwise given color
      ImU32 col              = ImGui::ColorConvertFloat4ToU32(v4Color) * (odd++ & 1);

      if (col == 0)
      {
        return;
      }
      float y1 = y0 + line_height;
      draw_list->AddRectFilled(ImVec2(x1, y0), ImVec2(x2, y1), col);
    }

    bool OutlinerWindow::DrawHeader(EntityPtr ntt, ImGuiTreeNodeFlags flags, int depth)
    {
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

      if (nextItemOpen)
      {
        ImGui::SetNextItemOpen(true);
      }
      // bright and dark color pattern for nodes. (even odd)
      DrawRowBackground(depth);

      const String sId = "##" + std::to_string(ntt->GetIdVal());
      // blue highlight for tree node on mouse hover
      ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.4f, 0.7f, 0.5f));
      ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.4f, 0.5f, 0.8f, 1.0f));
      bool isOpen = ImGui::TreeNodeEx(sId.c_str(), flags);
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
