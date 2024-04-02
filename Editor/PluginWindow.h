/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Window.h"

#include <EngineSettings.h>

namespace ToolKit
{
  namespace Editor
  {

    /**
     * Window that shows all the plugins in the project and their status.
     * Also allows load, unload and reload the plugins.
     */
    class PluginWindow : public Window
    {
     public:
      TKDeclareClass(PluginWindow, Window);

      PluginWindow();

      void Show() override;
      void ParsePluginDeclerations();

     private:
      PluginDeclerationArray m_plugins;
    };

    typedef std::shared_ptr<PluginWindow> PluginWindowPtr;

  } // namespace Editor
} // namespace ToolKit