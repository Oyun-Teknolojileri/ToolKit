#pragma once
#include "OverlayUI.h"

namespace ToolKit
{
  namespace Editor
  {

    class StatusBar : public OverlayUI
    {
     public:
      explicit StatusBar(EditorViewport* owner);
      void Show() override;
    };

  } // namespace Editor
} // namespace ToolKit
