/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

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