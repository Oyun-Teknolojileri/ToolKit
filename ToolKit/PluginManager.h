/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Plugin.h"
#include "Types.h"

namespace ToolKit
{

  // Platform dependent function callback decelerations.
  // Each platform suppose to fill these callbacks inside the plugin manager for functioning properly.
  typedef void* ModuleHandle;
  typedef GamePlugin*(__cdecl* FunctionAdress)();
  typedef std::function<ModuleHandle(StringView)> LoadModuleFn;
  typedef std::function<void(ModuleHandle)> FreeModuleFn;
  typedef std::function<void*(ModuleHandle, StringView)> GetFunctionFn;
  typedef std::function<String(const String&)> GetCreationTimeFn;

  struct PluginRegister
  {
    Plugin* m_plugin;
    ModuleHandle m_module;
    String m_lastWriteTime;
    String m_file;
    XmlDocumentPtr m_stateCache;
    bool m_loaded;
    bool m_initialized;
  };

  class TK_API PluginManager
  {
   public:
    PluginManager();
    ~PluginManager();

    /**
     * Loads the plugin from the given path. If the plugin has already loaded, check if update is needed.
     * Unloads the plugin with a provided state cache for plugin to cache its state. Than unloads the current plugin
     * and loads the new one with the cached state to restore plugin's state.
     * @param file is the path to the plugin.
     * @return true if the plugin is loaded.
     */
    bool Load(const String& file);

    /**
     * Unloads the given plugin. Passes the state cache inside the PluginRegister structure. If a reload is needed just
     * call the Load function.
     * @param file is the path to the plugin.
     */
    void Unload(const String& file);

    /**
     * Reloads the given plugin if it is dirty.
     * @return Reloaded plugin. The one sent as argument becomes invalid after reload.
     */
    Plugin* Reload(Plugin* plugin);

    /**
     * Updates the plugins with given delta time.
     * @param deltaTime is the elapsed seconds for the past frame.
     */
    void Update(float deltaTime);

    /**
     * Initialize plugin manager and loads all plugins for the current project.
     */
    void Init();

    /**
     * Uninitialize all plugins and the plugin manager.
     */
    void UnInit();

    /**
     * Returns the PluginRegister associated with the given file.
     * @param file is used to find associated PluginRegister.
     * @param indx is the index of the associated PluginRegister in the m_storage.
     */
    PluginRegister* GetRegister(const String& file, int* indx = nullptr);

    /**
     * Find and returns the PluginRegister that is associated with given plugin.
     * @param plugin is the given plugin to find associated register.
     * @return PluginRegister associated with given plugin if found or nullptr.
     */
    PluginRegister* GetRegister(const Plugin* plugin);

    GamePlugin* GetGamePlugin();
    void UnloadGamePlugin();

    LoadModuleFn LoadModule           = nullptr;
    FreeModuleFn FreeModule           = nullptr;
    GetFunctionFn GetFunction         = nullptr;
    GetCreationTimeFn GetCreationTime = nullptr;

   private:
    PluginRegister* GetGameRegister();

   public:
    std::vector<PluginRegister> m_storage;

   private:
    String m_pluginExtention; //!< System specific library extension.
  };

} // namespace ToolKit
