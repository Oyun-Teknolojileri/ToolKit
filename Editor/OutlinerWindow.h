#pragma once

#include "UI.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace ToolKit
{
  namespace Editor
  {
    class OutlinerWindow : public Window
    {
     public:
      explicit OutlinerWindow(XmlNode* node);
      OutlinerWindow();
      virtual ~OutlinerWindow();
      void Show() override;
      Type GetType() const override;
      void DispatchSignals() const override;
      void Focus(Entity* ntt);

     private:
      bool DrawRootHeader(const String& rootName,
                          uint id,
                          ImGuiTreeNodeFlags flags,
                          TexturePtr icon);

      void ShowSearchBar(String& searchString);
      bool DrawHeader(Entity* ntt, ImGuiTreeNodeFlags flags, int depth);
      
      int ShowNode(Entity* e, int depth);
      void DrawRowBackground(int depth);
      void SetItemState(Entity* e);
      void HandleSearch(const EntityRawPtrArray& ntties,
                        const EntityRawPtrArray& roots);

      void SelectEntitiesBetweenNodes(class EditorScene* scene,
                                      Entity* a,
                                      Entity* b);

      bool FindShownEntities(Entity* e, const String& str);
      void PushSelectedEntitiesToReparentQueue(Entity* parent);
      void TryReorderEntites(float treeStartY);
      void SortDraggedEntitiesByNodeIndex();

    private:
      /**
       * Focus uses this internal array, Show() opens all nodes and sets focus
       * to last ntt in the array.
       */
      EntityRawPtrArray m_nttFocusPath;
      std::unordered_set<Entity*> m_shownEntities;
      /**
       * entities up to down when we look at node tree.
       * these are imgui visible entities.
       */
      std::vector<Entity*> m_indexToEntity;

      EntityRawPtrArray m_draggingEntities;
      EntityRawPtrArray m_roots;
      Entity* m_lastClickedEntity = nullptr;
      Entity* m_rootsParent       = nullptr;
      
      String m_searchString   = "";
      bool m_stringSearchMode = false;
      bool m_searchCaseSens   = true;
      bool m_anyEntityHovered = false;
      // for even odd pattern
      int odd = 0;
    };

  } // namespace Editor
} // namespace ToolKit
