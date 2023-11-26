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

  enum class PluginType
  {
    Base,
    Engine,
    Game,
    Editor
  };

  typedef std::function<void(void)> PluginEventCallback;

  class TK_API Plugin
  {
   public:
    Plugin() {}

    virtual ~Plugin() {}

    virtual PluginType GetType()          = 0;

    /**
     * This function get called right after the plugin loaded.
     * Provides the host application's Main instance.
     */
    virtual void Init(class Main* master) = 0;

    /**
     * This function get called before the plugin unloaded from the memory.
     */
    virtual void Destroy()                = 0;

    /**
     * Update gets called every frame of the engine.
     * @param - deltaTime is the time elapsed between each frame in milliseconds.
     */
    virtual void Frame(float deltaTime)   = 0;

   protected:
    /**
     * Hot reload callback, this callback is get called after a reload. Gives you a chance to restore the state, if any
     * captured during an unload.
     */
    PluginEventCallback m_load;

    /**
     * Hot reload callback, this callback is get called before unload. Gives you a chance to store any state to restore
     * at reload.
     */
    PluginEventCallback m_unload;
  };

  class TK_API GamePlugin : public Plugin
  {
   public:
    /**
     * This function sets the viewport where the game will be played on. Promise in PIE session is that, it will be set
     * once before the game play begin and never gets changed during the PIE session.
     */
    virtual void SetViewport(Viewport* viewport) { m_viewport = viewport; };

    PluginType GetType() { return PluginType::Game; }

    bool GetQuitFlag() { return m_quit; }

   protected:
    /**
     * When set to true, game play stops, plugin gets unloaded. Can be used as a termination of PIE session.
     */
    bool m_quit          = false;

    /**
     * Viewport where the game played on.
     */
    Viewport* m_viewport = nullptr;
  };

} // namespace ToolKit
