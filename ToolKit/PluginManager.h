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
    bool m_loaded;
  };

  class TK_API PluginManager
  {
   public:
    PluginManager();
    ~PluginManager();

    // Platform dependent functions.
    bool Load(const String& file); // Auto reloads if the dll is dirty.
    void Unload(const String& file);
    bool Reload(const String& file);
    void Update(float deltaTime);

    // No platform dependency.
    void Init();
    void UnInit();
    PluginRegister* GetRegister(const String& file, int* indx = nullptr);

    // Shorts for game plugin.
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
  };

} // namespace ToolKit
