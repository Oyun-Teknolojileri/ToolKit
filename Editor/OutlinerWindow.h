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
      bool DrawHeader(Entity* ntt, ImGuiTreeNodeFlags flags, float depth);
      void ShowNode(Entity* e, float depth);
      void SetItemState(Entity* e);
      void HandleSearch(const EntityRawPtrArray& ntties,
                        const EntityRawPtrArray& roots);
      bool FindShownEntities(Entity* e, const String& str);

     private:
      /**
       * Focus uses this internal array, Show() opens all nodes and sets focus
       * to last ntt in the array.
       */
      EntityRawPtrArray m_nttFocusPath;
      std::unordered_map<Entity*, bool> m_shownEntities;
      String m_searchString   = "";
      bool m_stringSearchMode = false;
      bool m_searchCaseSens   = true;
    };

  } // namespace Editor
} // namespace ToolKit
