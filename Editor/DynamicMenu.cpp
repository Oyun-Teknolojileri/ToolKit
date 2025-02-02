/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "DynamicMenu.h"

#include "App.h"

#include <ObjectFactory.h>

namespace ToolKit
{
  namespace Editor
  {

    void DynamicMenu::AddSubMenuUnique(DynamicMenuPtr subMenu)
    {
      for (DynamicMenuPtr& subIt : SubMenuArray)
      {
        if (subIt->MenuName == subMenu->MenuName)
        {
          return;
        }
      }

      SubMenuArray.push_back(subMenu);
    }

    void ShowDynamicMenu(DynamicMenuPtr parentMenu)
    {
      if (DynamicMenuPtr menu = parentMenu)
      {
        if (ImGui::BeginMenu(menu->MenuName.c_str()))
        {
          for (DynamicMenuPtr& subMenu : menu->SubMenuArray)
          {
            ShowDynamicMenu(subMenu);
          }

          for (auto& menuEntry : menu->MenuEntries)
          {
            if (ImGui::MenuItem(menuEntry.second.c_str()))
            {
              EntityPtr entity = MakeNewPtrCasted<Entity>(menuEntry.first);
              g_app->GetCurrentScene()->AddEntity(entity);
            }
          }

          ImGui::EndMenu();
        }
      }
    }

    void ConstructDynamicMenu(StringArray menuDescriptors, DynamicMenuPtrArray& menuArray)
    {
      // Construct dynamic menu map.
      std::unordered_map<String, DynamicMenuPtr> menuMap;

      auto errContinueFn = [](String& metaVal) -> void { TK_WRN("Wrong menu descriptor format: %s", metaVal); };

      for (String& descriptor : menuDescriptors)
      {
        StringArray parts;
        Split(descriptor, "/", parts);

        if (parts.size() < 2)
        {
          errContinueFn(descriptor);
          continue;
        }

        StringArray classNamePair;
        Split(parts.back(), ":", classNamePair);

        if (classNamePair.size() != 2)
        {
          errContinueFn(descriptor);
          continue;
        }

        // Loop over menus and fill its entries.
        String menu;
        for (int i = 0; i < (int) parts.size() - 1; i++)
        {
          menu       += parts[i];
          auto entry  = menuMap.find(menu);

          if (entry == menuMap.end()) // If menu has not been created, create it.
          {
            DynamicMenuPtr menuPtr = std::make_shared<DynamicMenu>();
            menuPtr->MenuName      = parts[i];
            menuMap[menu]          = menuPtr;

            if (i == 0) // If this is the first entry, its a root menu.
            {
              menuArray.push_back(menuPtr);
            }
          }

          DynamicMenuPtr menuPtr = menuMap[menu];
          if (i == (int) parts.size() - 2) // If its the last section, it contains the value pair.
          {
            menuPtr->MenuEntries.push_back({classNamePair[0], classNamePair[1]});
          }

          // Chain the menus.
          if (i > 0) // If this is not the root, it must be chained.
          {
            String parentMenu;
            for (int j = 0; j < i; j++)
            {
              parentMenu += parts[j];
            }

            auto parentMenuIt = menuMap.find(parentMenu);
            if (parentMenuIt != menuMap.end())
            {
              parentMenuIt->second->AddSubMenuUnique(menuMap[menu]);
            }
          }
        }
      }
    }

  } // namespace Editor
} // namespace ToolKit
