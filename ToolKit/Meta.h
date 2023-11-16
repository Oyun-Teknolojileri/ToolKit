/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

namespace ToolKit
{
  /**
   * MetaKey for Editor to display custom object types registered by plugins.
   * Value Pattern: Menu/SubMenu/Class:Name
   * Menu/SubMenu will appear this way in the target menu.
   * Class will be used to construct object. So it must be T::StaticClass()->Name
   * Name will be used to display name for the object.
   */
  constexpr StringView MenuMetaKey = "MenuMetaKey";

} // namespace ToolKit
