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

    class ComponentView : public View
    {
     public:
      static void ShowAnimControllerComponent(ParameterVariant* var, ComponentPtr comp);
      static bool ShowComponentBlock(ComponentPtr& comp, const bool modifiableComp);
      ComponentView();
      virtual ~ComponentView();
      virtual void Show();
    };

  } // namespace Editor
} // namespace ToolKit