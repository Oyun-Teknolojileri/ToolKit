/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "EditorApi.h"

#include "App.h"

namespace ToolKit
{
  namespace Editor
  {

#define CheckEditor                                                                                                    \
  if (g_app == nullptr)                                                                                                \
    return;

    void AddToMenu(StringView menuDescriptor)
    {
      CheckEditor;

      bool exist = false;
      for (String& meta : g_app->m_customObjectMetaValues)
      {
        if (meta == menuDescriptor)
        {
          exist = true;
          break;
        }
      }

      if (!exist)
      {
        g_app->m_customObjectMetaValues.push_back(String(menuDescriptor));
      }
    }

    void RemoveFromMenu(StringView menuDescriptor)
    {
      CheckEditor;

      for (int i = (int) g_app->m_customObjectMetaValues.size() - 1; i >= 0; i--)
      {
        if (g_app->m_customObjectMetaValues[i] == menuDescriptor)
        {
          g_app->m_customObjectMetaValues.erase(g_app->m_customObjectMetaValues.begin() + i);
        }
      }
    }

    TK_EDITOR_API void UpdateDynamicMenus()
    {
      CheckEditor;

      g_app->ReconstructDynamicMenus();
    }

  } // namespace Editor
} // namespace ToolKit
