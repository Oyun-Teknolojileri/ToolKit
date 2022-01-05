#pragma once

#include "Types.h"
#include "Plugin.h"
#include <functional>

namespace ToolKit
{

  struct PluginRegister
  {
    Plugin* m_plugin;
    void* m_module;
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
    void UnInit();
    bool Load(const String& file); // Auto reloads if the dll is dirty.
    
    // No platform dependency.
    void Init();
    PluginRegister* GetRegister(const String& file);
    void Unload(const String& file);
    void Report(const char* msg, ...);

    // Shorts for game plugin.
    GamePlugin* GetGamePlugin();
    void UnloadGamePlugin();

  private:
    PluginRegister* GetGameRegister();

  public:
    std::vector<PluginRegister> m_storage;
    std::function<void(const String&)> m_reporterFn;
  };

}
