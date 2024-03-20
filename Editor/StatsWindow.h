/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once
#include "UI.h"

namespace ToolKit
{
  namespace Editor
  {
    class StatsWindow : public Window
    {
     public:
      TKDeclareClass(StatsWindow, Window);

      StatsWindow();
      virtual ~StatsWindow();
      virtual void Show();
    };

    typedef std::shared_ptr<StatsWindow> StatsWindowPtr;

  } // namespace Editor
} // namespace ToolKit
