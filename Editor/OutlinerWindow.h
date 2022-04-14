#pragma once

#include "UI.h"


namespace ToolKit
{
  namespace Editor
  {
    class OutlinerWindow : public Window
    {
      
    public:
      OutlinerWindow(XmlNode* node);
      OutlinerWindow();
      virtual ~OutlinerWindow();
      virtual void Show() override;
      virtual Type GetType() const override;
      virtual void DispatchSignals() const override;
      void Focus(Entity* ntt);

    private:
      bool DrawHeader(const String& text, uint id, ImGuiTreeNodeFlags flags, TexturePtr icon);
      bool DrawHeader(Entity* ntt, ImGuiTreeNodeFlags flags);
      void ShowNode(Entity* e);
      void SetItemState(Entity* e);

    private:
      EntityRawPtrArray m_nttFocusPath; // Focus uses this internal array, Show() opens all nodes and sets focus to last ntt in the array.
    };
  }
}
