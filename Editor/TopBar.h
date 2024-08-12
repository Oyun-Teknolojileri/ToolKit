/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

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
      static void ShowAddMenuPopup();

     protected:
      void ShowAddMenu(std::function<void()> showMenuFn, uint& nextItemIndex);
      void ShowTransformOrientation(uint& nextColumnItem);
      void SnapOptions(uint& nextItemIndex);
      void CameraAlignmentOptions(uint& nextItemIndex);
    };

  } // namespace Editor
} // namespace ToolKit