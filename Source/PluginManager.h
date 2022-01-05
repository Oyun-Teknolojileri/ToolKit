#pragma once

#include "Types.h"
#include "Plugin.h"
#include <functional>

namespace ToolKit
{
  class TK_API PluginManager
  {
  public:
    PluginManager();
    ~PluginManager();

    void Init();
    void UnInit();
    bool Load(const String& plugin);
    void Unload();

  public:
    GamePlugin* m_plugin = nullptr;
    void* m_moduleHandle = nullptr;
    std::function<void(const String&)> m_reporterFn;
  };
}
