/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

/**
 * Plugin functions that needs to be accessible by the editor needs to export their functionality using this macro.
 */
#ifdef _WIN32 // Windows.
  #define TK_PLUGIN_API __declspec(dllexport)
#else // Other OS.
  #define TK_PLUGIN_API
#endif

namespace ToolKit
{

  /**
   * Enums for plugin types.
   */
  enum class PluginType
  {
    Base,
    /**
     * Plugin type that is used to extend engine features.
     */
    Engine,
    /**
     * Plugin type for games and simulations.
     */
    Game,
    /**
     * Plugin type for extending the editor.
     */
    Editor
  };

  /**
   * Enums for plugin states.
   */
  enum class PluginState
  {
    /**
     * Plugin at this state gets its Init function called. Afterwards its OnLoad gets called.
     */
    Started,
    /**
     * Plugin at this state gets its Frame executed at every frame.
     */
    Running,
    /**
     * Plugin at this state stop receiving Frame updates. But it still alive and can continue from running exactly where
     * it left. Basically, it doesn't get Frame updates. Scene and animation updates doesn't occur.
     */
    Paused,
    /**
     * Plugin at this state is stopped and its Destroy called. When started again its Init function gets called. Be
     * wary, unless otherwise stated all plugins are loaded. So even its Destroy called its still loaded.
     */
    Stop
  };

  /**
   * Base plugin interface.
   */
  class TK_API Plugin
  {
   public:
    Plugin() {}

    virtual ~Plugin() {}

    virtual PluginType GetType()                = 0;

    /**
     * This function get called right after the plugin loaded.
     * Provides the host application's Main instance.
     */
    virtual void Init(class Main* master)       = 0;

    /**
     * This function get called before the plugin unloaded from the memory.
     */
    virtual void Destroy()                      = 0;

    /**
     * Update gets called every frame of the engine.
     * @param - deltaTime is the time elapsed between each frame in milliseconds.
     */
    virtual void Frame(float deltaTime)         = 0;

    /**
     * This callback is get called after a reload. Gives you a chance to restore the state, if any
     * captured during an unload.
     */
    virtual void OnLoad(XmlDocumentPtr state)   = 0;

    /**
     * This callback is get called before unload. Gives you a chance to store any state to restore
     * at reload.
     */
    virtual void OnUnload(XmlDocumentPtr state) = 0;

   public:
    PluginState m_currentState = PluginState::Stop;
  };

  /**
   * Plugin type that can be used for simulation, games and other interactive applications.
   */
  class TK_API GamePlugin : public Plugin
  {
   public:
    /**
     * This function sets the viewport where the game will be played on. Promise in PIE session is that, it will be set
     * once before the game play begin and never gets changed during the PIE session.
     */
    virtual void SetViewport(ViewportPtr viewport) { m_viewport = viewport; };

    /**
     * Returns plugin type.
     * @return PluginType::Game.
     */
    PluginType GetType() { return PluginType::Game; }

    /**
     * This callback gets called when the play session started for the first time.
     */
    virtual void OnPlay()   = 0;

    /**
     * This callback gets called when the play session stopped.
     */
    virtual void OnPause()  = 0;

    /**
     * This callback gets called play session continue after paused.
     */
    virtual void OnResume() = 0;

    /**
     * This callback gets called play session continue stopped.
     */
    virtual void OnStop()   = 0;

   protected:
    /**
     * Viewport where the game played on.
     */
    ViewportPtr m_viewport = nullptr;
  };

} // namespace ToolKit
