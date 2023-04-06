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

#include <vector>

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
    std::vector<ULongID> g_child;

    void DrawTreeNodeLine(int numNodes, ImVec2 rectMin)
    {
      const ImColor color  = ImGui::GetColorU32(ImGuiCol_Text);
      ImDrawList* drawList = ImGui::GetWindowDrawList();
      ImVec2 cursorPos     = ImGui::GetCursorScreenPos();
      float item_spacing_y = ImGui::GetStyle().ItemSpacing.y;
      float line_height    = ImGui::GetTextLineHeight() + item_spacing_y;
      float halfHeight     = line_height * 0.5f;

      // -11 align line with arrow
      rectMin.x        = cursorPos.x - 11.0f;
      rectMin.y       += halfHeight;

      float bottom = rectMin.y + (numNodes * line_height);
      bottom -= (halfHeight + 1.0f); // move up a little

      drawList->AddLine(rectMin, ImVec2(rectMin.x, bottom), color);
      // a little bulge at the end of the line
      drawList->AddLine(ImVec2(rectMin.x, bottom),
                        ImVec2(rectMin.x + 5.0f, bottom),
                        color);
    }

    // returns total drawed nodes
    int OutlinerWindow::ShowNode(Entity* e, int depth)
    {
      if (!m_shownEntities[e])
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

    // this algorithm will walk from down to up and add to selection 
    void OutlinerWindow::SelectNodesRec(EditorScene* scene,
                                        Entity* ntt,
                                        Entity* target,
                                        int childIdx)
    {
      if (m_multiSelectComplated || ntt == nullptr)
      {
        return;
      }
      
      bool hasParent = ntt->m_node->m_parent == nullptr;

      if (hasParent == false)
      {
        scene->AddToSelection(ntt->GetIdVal(), true);
        return;
      }

      Node* parent = ntt->m_node->m_parent;
      
      if (childIdx == -1) // unknown?
      {
        // find index of the node
        for (childIdx = 0ull; 
             childIdx < parent->m_children.size(); 
           ++childIdx)
        { 
          if (parent->m_children[childIdx]->m_entity == ntt)
          {
            break;
          }
        }
        // can't we found ourselfs in parent? shouldn't happen.
        if (childIdx == parent->m_children.size())
        {
          assert(false && "child is not exist in parent's array");
          return;
        }
      }

      for (size_t i = 0ull; i <= childIdx; ++i)
      {
        scene->AddToSelection(parent->m_children[i]->m_entity->GetIdVal(),
                              true);
      }

      // we may want to add childrens of childrens as well but for now just parents
      SelectNodesRec(scene, parent->m_entity, target, childIdx--);
    }

    void OutlinerWindow::SelectEntitiesBetweenNodes(EditorScene* scene,
                                                    Entity* a, Entity* b)
    {
      if (a == b)
      {
        return;
      }

      // todo: selected entities should be siblings or both a and b should be root nodes
      //       otherwise we may want to detect and early exit from this process(with warning)

      bool bHasParent = b->m_node->m_parent != nullptr;
      bool aHasParent = a->m_node->m_parent != nullptr;

      if (a->m_node->m_parent != b->m_node->m_parent)
      {
        GetLogger()->WriteConsole(LogType::Warning, 
                                  "selected entities should have same parent!");
        return;
      }

      // find locations of a and b on m_roots
      size_t aIndex = 0ull, bIndex = 0ull;
      size_t rootIdx = 0ull, foundCnt = 0;

      for (; rootIdx < m_roots.size(); ++rootIdx)
      {
        size_t aFound = m_roots[rootIdx] == a;
        size_t bFound = m_roots[rootIdx] == b;

        aIndex = aFound ? rootIdx : aIndex;
        bIndex = bFound ? rootIdx : bIndex;

        foundCnt += aFound + bFound;

        if (foundCnt == 2) // found both of them?
        { 
          Entity* lastOccur = m_roots[rootIdx];
          if (lastOccur == b)
          {
            // we want to go from a to b
            std::swap(a, b);
            std::swap(aIndex, bIndex);
          }
          break;
        }
      }

      // can't find any of the nodes ? (shouldn't happen)
      if (rootIdx == m_roots.size() || foundCnt < 2)
      {
        return;
      }
      // going upwards and selecting entities
      // entityA <
      //         ^
      // entityB ^
      while (rootIdx >= bIndex)
      {
        scene->AddToSelection(m_roots[rootIdx]->GetIdVal(), true);
        rootIdx--;
      }

      // start recursive selecting algorithm
      // m_multiSelectComplated = false;
      // SelectNodesRec(scene, a, b, -1);
    }

    void OutlinerWindow::SetItemState(Entity* e)
    {
      EditorScenePtr currScene = g_app->GetCurrentScene();
      bool pressingShift = ImGui::IsKeyDown(ImGuiKey_LeftShift);

      if (ImGui::IsItemClicked())
      {
        if (pressingShift)
        {
          if (m_lastClickedEntity != nullptr)
          {
            SelectEntitiesBetweenNodes(currScene.get(), m_lastClickedEntity, e);
          }
          else if (currScene->IsSelected(e->GetIdVal()))
          {
            currScene->RemoveFromSelection(e->GetIdVal());
          }
          else
          {
            currScene->AddToSelection(e->GetIdVal(), true);
          }
        }
        else
        {
          if (!currScene->IsSelected(e->GetIdVal()))
          {
            currScene->AddToSelection(e->GetIdVal(), false);
            g_app->GetPropInspector()->m_activeView =
                PropInspector::ViewType::Entity;
          }
        }
        m_lastClickedEntity = e;
      }
      
      if (ImGui::BeginDragDropSource())
      {
        ImGui::SetDragDropPayload("HierarcyChange", nullptr, 0);
        ImGui::Text("Drop on the new parent.");
        ImGui::EndDragDropSource();
      }

      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("HierarcyChange"))
        {
          // Change the selected files hierarchy
          EntityRawPtrArray selected;
          currScene->GetSelectedEntities(selected);

          if (e->GetType() != EntityType::Entity_Prefab)
          {
            for (int i = 0; i < selected.size(); i++)
            {
              if (selected[i]->GetIdVal() != e->GetIdVal() &&
                  (!Prefab::GetPrefabRoot(selected[i]) ||
                   selected[i]->GetType() == EntityType::Entity_Prefab))
              {
                g_child.push_back(selected[i]->GetIdVal());
              }
            }
          }
          g_parent = e->GetIdVal();
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
        m_shownEntities[ntt] = m_searchString.empty();
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

      bool result        = self || children;
      m_shownEntities[e] = result;
      return result;
    }

    void OutlinerWindow::Show()
    {
      EditorScenePtr currScene = g_app->GetCurrentScene();

      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        odd = 0;
        HandleStates();
        ShowSearchBar(m_searchString);
        ImGui::BeginChild("##Outliner Nodes");
        ImGuiTreeNodeFlags flag =
            g_treeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen;

        if (DrawRootHeader("Scene", 0, flag, UI::m_collectionIcon))
        {
          const EntityRawPtrArray& ntties = currScene->GetEntities();
          m_roots.clear();

          GetRootEntities(ntties, m_roots);
          HandleSearch(ntties, m_roots);

          for (size_t i = 0; i < m_roots.size(); ++i)
          {
            ShowNode(m_roots[i], 0);
          }

          ImGui::TreePop();
        }

        ImGui::EndChild();
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
              g_child.push_back(selected[i]->GetIdVal());
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

      m_visibleEntites[ntt] = isOpen;

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
  