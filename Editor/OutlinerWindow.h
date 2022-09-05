#pragma once

#include "UI.h"

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

      bool DrawHeader(Entity* ntt, ImGuiTreeNodeFlags flags);
      void ShowNode(Entity* e);
      void SetItemState(Entity* e);

     private:
      /**
       * Focus uses this internal array, Show() opens all nodes and sets focus
       * to last ntt in the array.
       */
      EntityRawPtrArray m_nttFocusPath;
    };

  } // namespace Editor
} // namespace ToolKit
