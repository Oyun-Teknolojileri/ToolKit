/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

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
      void Show2DViewZoomOptions(uint& nextItemIndex);
      void ShowGridOptions(uint& nextItemIndex);
    };

  } // namespace Editor
} // namespace ToolKit