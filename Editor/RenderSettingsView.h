#pragma once
#include "PropInspector.h"

namespace ToolKit
{
  namespace Editor
  {
    class RenderSettingsView : public Window
    {
     public:
      RenderSettingsView();
      virtual ~RenderSettingsView();
      virtual void Show();
      Type GetType() const override;
    };
  } // namespace Editor
} // namespace ToolKit