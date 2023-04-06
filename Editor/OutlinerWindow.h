#pragma once

#include "UI.h"

#include <string>
#include <unordered_map>

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

      void SelectNodesRec(class EditorScene* scene,
                          Entity* ntt,
                          Entity* target,
                          int childIdx);

      void SelectEntitiesBetweenNodes(class EditorScene* scene,
                                      Entity* a,
                                      Entity* b);

      bool FindShownEntities(Entity* e, const String& str);
     
    private:
      /**
       * Focus uses this internal array, Show() opens all nodes and sets focus
       * to last ntt in the array.
       */
      EntityRawPtrArray m_nttFocusPath;
      std::unordered_map<Entity*, bool> m_shownEntities;
      EntityRawPtrArray m_roots;
      Entity* m_lastClickedEntity = nullptr;
      Entity* m_rootsParent       = nullptr;
      /**
       * Visible from UI
       */
      std::unordered_map<Entity*, bool> m_visibleEntites;
      
      String m_searchString   = "";
      bool m_stringSearchMode = false;
      bool m_searchCaseSens   = true;
      // for even odd pattern
      int odd = 0;
      bool m_multiSelectComplated = false;
    };

  } // namespace Editor
} // namespace ToolKit
