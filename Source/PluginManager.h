#pragma once

#include "Types.h"
#include "Plugin.h"
#include <functional>

namespace ToolKit
{
  class PluginManager
  {
  public:
    ~PluginManager();

    void Init();
    void UnInit();
    void Load(const String& plugin);
    void Unload();
    static PluginManager* GetInstance();

  private:
    PluginManager();

  public:
    GamePlugin* m_plugin = nullptr;
    void* m_moduleHandle = nullptr;
    std::function<void(const String&)> m_reporterFn;

  private:
    static PluginManager* m_instance;
  };
}
