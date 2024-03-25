/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Window.h"

namespace ToolKit
{
  namespace Editor
  {

    class RenderSettingsWindow : public Window
    {
     public:
      TKDeclareClass(RenderSettingsWindow, Window);

      RenderSettingsWindow();
      virtual ~RenderSettingsWindow();
      void Show() override;
    };

    typedef std::shared_ptr<RenderSettingsWindow> RenderSettingsWindowPtr;

  } // namespace Editor
} // namespace ToolKit