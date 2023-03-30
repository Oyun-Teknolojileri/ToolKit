#pragma once
#include "OverlayUI.h"

namespace ToolKit
{
  namespace Editor
  {

    class OverlayLeftBar : public OverlayUI
    {
     public:
      explicit OverlayLeftBar(EditorViewport* owner);
      void Show() override;
    };

  } // namespace Editor
} // namespace ToolKit
