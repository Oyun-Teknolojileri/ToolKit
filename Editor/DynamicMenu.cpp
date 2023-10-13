/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "DynamicMenu.h"

#include "App.h"

#include <Meta.h>
#include <ObjectFactory.h>

#include <DebugNew.h>

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
      TKMap<String, DynamicMenuPtr> menuMap;

      auto errContinueFn = [](String& metaVal) -> void
      { GetLogger()->WriteConsole(LogType::Warning, "%s value is wrong: %s", MenuMetaKey, metaVal); };

      for (String& customObjectMetaVal : menuDescriptors)
      {
        StringArray parts;
        Split(customObjectMetaVal, "/", parts);

        if (parts.size() < 2)
        {
          errContinueFn(customObjectMetaVal);
          continue;
        }

        StringArray classNamePair;
        Split(parts.back(), ":", classNamePair);

        if (classNamePair.size() != 2)
        {
          errContinueFn(customObjectMetaVal);
          continue;
        }

        // Loop over menus and fill its entries.
        String menu;
        for (int i = 0; i < (int) parts.size() - 1; i++)
        {
          menu       += parts[i];
          auto entry = menuMap.find(menu);

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
