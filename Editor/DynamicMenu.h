/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

namespace ToolKit
{
  namespace Editor
  {

    typedef std::shared_ptr<struct DynamicMenu> DynamicMenuPtr;
    typedef std::vector<DynamicMenuPtr> DynamicMenuPtrArray;

    struct DynamicMenu
    {
      String MenuName;
      std::vector<std::pair<String, String>> MenuEntries; //!< Class - Name pairs.
      DynamicMenuPtrArray SubMenuArray;                   //!< SubMenu array of the menu.

      void AddSubMenuUnique(DynamicMenuPtr subMenu);
    };

    extern void ShowDynamicMenu(DynamicMenuPtr parentMenu);
    extern void ConstructDynamicMenu(StringArray menuDescriptors, DynamicMenuPtrArray& menuArray);

  } // namespace Editor
} // namespace ToolKit