/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "ToolKit.h"

#ifdef _WIN32
  #if defined(TK_EDITOR_DLL_EXPORT)
    #define TK_EDITOR_API __declspec(dllexport)
  #else
    #define TK_EDITOR_API __declspec(dllimport)
  #endif
#endif

namespace ToolKit
{
  namespace Editor
  {

    /**
     * @brief Add a new menu entry to Editor for a registered class.
     * This function allows plugins to add new classes to the Editor's menu system. The classes are added according to
     * the specified menu descriptor pattern.
     *
     * @param menuDescriptor A string that follows the pattern "Menu/SubMenu/Class:Name". The "Menu/SubMenu" defines the
     * location in the target menu where the class will appear. The "Class" is the identifier used to construct the
     * object, typically obtained from T::StaticClass()->Name. The "Name" is the display name for the object in the
     * menu.
     */
    TK_EDITOR_API void AddToMenu(StringView menuDescriptor);

    /**
     * @brief Removes the menu entry from Editor.
     * @param menuDescriptor A StringView representing the menu entry to be removed.
     */
    TK_EDITOR_API void RemoveFromMenu(StringView menuDescriptor);

    /**
     * @brief Updates the Editor's dynamic menus to reflect recent modifications.
     * This function should be called whenever changes are made to the menu structure or content. It ensures that the
     * Editor's menus display the most current information and options available to the user.
     */
    TK_EDITOR_API void UpdateDynamicMenus();

  } // namespace Editor
} // namespace ToolKit