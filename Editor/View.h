/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "DirectoryEntry.h"
#include "Types.h"

namespace ToolKit
{
  namespace Editor
  {

    enum class ViewType
    {
      Entity,
      CustomData,
      Component,
      Material,
      Mesh,
      Prefab,
      ViewCount
    };

    class View
    {
     public:
      View(const StringView viewName);
      virtual ~View();
      virtual void Show() = 0;

      // If isEditable = false;
      //  You should call EndDisabled() before using DropZone & DropSubZone
      static void DropZone(uint fallbackIcon,
                           const String& file,
                           std::function<void(DirectoryEntry& entry)> dropAction,
                           const String& dropName = "",
                           bool isEditable        = true);

      static void DropSubZone(const String& title,
                              uint fallbackIcon,
                              const String& file,
                              std::function<void(const DirectoryEntry& entry)> dropAction,
                              bool isEditable = true);

      static bool IsTextInputFinalized();

     public:
      EntityWeakPtr m_entity;
      int m_viewID         = 0;
      TexturePtr m_viewIcn = nullptr;
      StringView m_fontIcon;
      const StringView m_viewName;
    };

    typedef View* ViewRawPtr;
    typedef std::vector<ViewRawPtr> ViewRawPtrArray;

  } // namespace Editor
} // namespace ToolKit