#pragma once

#include "OverlayUI.h"

namespace ToolKit
{
  namespace Editor
  {

    class OverlayTopBar : public OverlayUI
    {
     public:
      explicit OverlayTopBar(EditorViewport* owner);
      void Show() override;

     protected:
      void ShowAddMenu(std::function<void()> showMenuFn,
                       uint32_t& nextItemIndex);
      void ShowTransformOrientation(uint32_t& nextColumnItem);
      void SnapOptions(uint32_t& nextItemIndex);
      void CameraAlignmentOptions(uint32_t& nextItemIndex);
      void Show2DViewZoomOptions(uint32_t& nextItemIndex);
      void ShowGridOptions(uint32_t& nextItemIndex);
    };

  } // namespace Editor
} // namespace ToolKit