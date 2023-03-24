#pragma once

#include "TopBar.h"

namespace ToolKit
{
  namespace Editor
  {

    class Overlay2DTopBar : public OverlayTopBar
    {
     public:
      explicit Overlay2DTopBar(EditorViewport* owner);
      void Show() override;

     protected:
      void Show2DViewZoomOptions(uint32_t& nextItemIndex);
      void ShowGridOptions(uint32_t& nextItemIndex);
    };

  } // namespace Editor
} // namespace ToolKit