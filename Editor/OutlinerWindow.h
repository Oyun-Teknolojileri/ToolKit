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

    private:
      bool DrawHeader(const String& text, uint id, ImGuiTreeNodeFlags flags, TexturePtr icon);
      bool DrawHeader(Entity* ntt, ImGuiTreeNodeFlags flags);
      void ShowNode(Entity* e);
    };
  }
}
