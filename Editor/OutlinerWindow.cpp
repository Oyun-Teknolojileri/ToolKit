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

    static int odd = 0;

    // customized version of this: https://github.com/ocornut/imgui/issues/2668
    void DrawRowBackground(float depth)
    {
      float x1 = ImGui::GetCurrentWindow()->WorkRect.Min.x + (depth * 14.0f);
      float x2 = ImGui::GetCurrentWindow()->WorkRect.Max.x;
      float item_spacing_y  = ImGui::GetStyle().ItemSpacing.y;
      float item_offset_y   = -item_spacing_y * 0.5f;
      float line_height     = ImGui::GetTextLineHeight() + item_spacing_y;

      ImDrawList* draw_list = ImGui::GetWindowDrawList();
      float y0          = ImGui::GetCursorScreenPos().y + (float) item_offset_y;
      ImGuiStyle& style = ImGui::GetStyle();
      ImVec4 v4Color    = style.Colors[ImGuiCol_TabHovered];
      v4Color.x *= 0.8f;
      v4Color.y *= 0.8f;
      v4Color.z *= 0.8f;
      // if odd black otherwise given color
      ImU32 col  = ImGui::ColorConvertFloat4ToU32(v4Color) * (odd++ & 1);

      if ((col & IM_COL32_A_MASK) == 0)
      {
        return;
      }
      float y1 = y0 + line_height;
      draw_list->AddRectFilled(ImVec2(x1, y0), ImVec2(x2, y1), col);
    }

    void OutlinerWindow::ShowNode(Entity* e, float depth)
    {
      if (!m_shownEntities[e])
      {
        return;
      }

      ImGuiTreeNodeFlags nodeFlags = g_treeNodeFlags;
      EditorScenePtr currScene     = g_app->GetCurrentScene();
      if (currScene->IsSelected(e->GetIdVal()))
      {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
      }

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
          for (Node* n : e->m_node->m_children)
          {
            Entity* childNtt = n->m_entity;
            if (childNtt != nullptr)
            {
              ShowNode(childNtt, depth + 1.0f);
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
          if (currScene->IsSelected(e->GetIdVal()))
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
        if (m_searchString.empty())
        {
          m_shownEntities[ntt] = true;
        }
        else
        {
          m_shownEntities[ntt] = false;
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
            children   = child | children;
          }
        }
      }

      bool result        = self | children;
      m_shownEntities[e] = result;
      return result;
    }

    void OutlinerWindow::Show()
    {
      EditorScenePtr currScene = g_app->GetCurrentScene();
      ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, g_indentSpacing);

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
          EntityRawPtrArray roots;
          GetRootEntities(ntties, roots);
          HandleSearch(ntties, roots);

          for (Entity* e : roots)
          {
            ShowNode(e, 0.0f);
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

      ImGui::PopStyleVar();
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

    bool OutlinerWindow::DrawHeader(Entity* ntt,
                                    ImGuiTreeNodeFlags flags,
                                    float depth)
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

      DrawRowBackground(depth);

      const String sId = "##" + std::to_string(ntt->GetIdVal());
      bool isOpen      = ImGui::TreeNodeEx(sId.c_str(), flags);

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
      String icon      = ICON_FA_CUBE ICON_SPACE;
      EntityType eType = ntt->GetType();

      static std::unordered_map<EntityType, const char*> EntityTypeToIconMap = {
          {EntityType::Entity_Camera,           ICON_FA_VIDEO_CAMERA ICON_SPACE},
          {EntityType::Entity_AudioSource,      ICON_FA_FILE_AUDIO ICON_SPACE  },
          {EntityType::Entity_Node,             ICON_FA_ARROWS ICON_SPACE      },
          {EntityType::Entity_Prefab,           ICON_FA_CUBES ICON_SPACE       },

          {EntityType::Entity_Light,            ICON_FA_LIGHTBULB ICON_SPACE   },
          {EntityType::Entity_PointLight,       ICON_FA_LIGHTBULB ICON_SPACE   },
          {EntityType::Entity_SpotLight,        ICON_FA_LIGHTBULB ICON_SPACE   },
          {EntityType::Entity_DirectionalLight, ICON_FA_SUN ICON_SPACE         },

          {EntityType::Entity_Sky,              ICON_FA_SKYATLAS ICON_SPACE    },
          {EntityType::Entity_GradientSky,      ICON_FA_SKYATLAS ICON_SPACE    },
      };

      auto entityIcon = EntityTypeToIconMap.find(eType);
      if (entityIcon != EntityTypeToIconMap.end())
      {
        icon = entityIcon->second;
      }

      ImGui::SameLine();
      ImGui::Text((icon + ntt->GetNameVal()).c_str());

      // Hiearchy visibility
      float offset = ImGui::GetContentRegionAvail().x - 45.0f;
      ImGui::SameLine(offset);
      icon = ntt->GetVisibleVal() ? ICON_FA_EYE : ICON_FA_EYE_SLASH;

      // Texture only toggle button.
      ImGui::PushID(static_cast<int>(ntt->GetIdVal()));
      if (UI::ButtonDecorless(icon, ImVec2(18.0f, 15.0f), false))
      {
        ntt->SetVisibility(!ntt->GetVisibleVal(), true);
      }
      ImGui::PopID();

      offset = ImGui::GetContentRegionAvail().x - 20.0f;
      ImGui::SameLine(offset);
      icon = ntt->GetTransformLockVal() ? ICON_FA_LOCK : ICON_FA_UNLOCK;

      // Texture only toggle button.
      ImGui::PushID(static_cast<int>(ntt->GetIdVal()));
      if (UI::ButtonDecorless(icon, ImVec2(18.0f, 15.0f), false))
      {
        ntt->SetTransformLock(!ntt->GetTransformLockVal(), true);
      }

      ImGui::PopID();

      return isOpen;
    }

  } // namespace Editor
} // namespace ToolKit
