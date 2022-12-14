#pragma once
#include "PropInspector.h"

namespace ToolKit
{
  namespace Editor
  {
    class RenderSettingsView : public View
    {
     public:
      RenderSettingsView();
      virtual ~RenderSettingsView();
      virtual void Show();
    };
  } // namespace Editor
} // namespace ToolKit