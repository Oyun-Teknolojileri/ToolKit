/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "PluginManager.h"

#include "Logger.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  bool PluginManager::Load(const String& file)
  {
    String dllName = file;

#ifdef TK_DEBUG
    dllName += "_d.dll";
#else
    dllName += ".dll";
#endif

    // Check the register.
    bool newRegAdded    = false;
    PluginRegister* reg = nullptr;
    if (reg = GetRegister(dllName))
    {
      // Unload if dirty
      if (!reg->m_loaded)
      {
        // Get file creation time.
        String cTime = GetCreationTime(dllName);
        if (reg->m_lastWriteTime != cTime)
        {
          TK_LOG("A new version of the plugin has been found.");

          // Reload.
          Unload(reg->m_file);
          if (reg->m_module)
          {
            FreeModule(reg->m_module);
          }
        }
      }
    }
    else
    {
      // A new reg is needed.
      newRegAdded = true;
      m_storage.push_back(PluginRegister());
      reg = &m_storage.back();
    }

    ModuleHandle module = LoadModule(dllName.c_str());
    if (module != nullptr)
    {
      reg->m_module           = module;
      FunctionAdress ProcAddr = (FunctionAdress) GetFunction(module, "CreateInstance");

      if (ProcAddr != nullptr)
      {
        reg->m_plugin = (ProcAddr) ();
        reg->m_plugin->Init(Main::GetInstance());
        reg->m_loaded        = true;
        reg->m_file          = dllName;
        reg->m_lastWriteTime = GetCreationTime(dllName);
        return true;
      }
      else
      {
        TK_ERR("Can not find CreateInstance() in the plugin.");
      }
    }
    else
    {
      TK_ERR("Can not load plugin %s", dllName.c_str());
    }

    if (newRegAdded)
    {
      m_storage.pop_back();
    }

    return false;
  }

  void PluginManager::UnInit()
  {
    for (PluginRegister& reg : m_storage)
    {
      Unload(reg.m_file);
    }

    m_storage.clear();
  }

  void PluginManager::Unload(const String& file)
  {
    PluginRegister* reg = GetRegister(file);
    if (reg == nullptr || reg->m_plugin == nullptr || !reg->m_loaded)
    {
      return;
    }

    reg->m_plugin->Destroy();
    reg->m_plugin = nullptr;

    if (reg->m_module)
    {
      FreeModule(reg->m_module);
      reg->m_module = nullptr;
    }

    reg->m_loaded = false;
  }

  GamePlugin* PluginManager::GetGamePlugin()
  {
    if (PluginRegister* reg = GetGameRegister())
    {
      return static_cast<GamePlugin*>(reg->m_plugin);
    }

    return nullptr;
  }

  void PluginManager::UnloadGamePlugin()
  {
    if (PluginRegister* reg = GetGameRegister())
    {
      Unload(reg->m_file);
    }
  }

  PluginRegister* PluginManager::GetGameRegister()
  {
    for (size_t i = 0; i < m_storage.size(); i++)
    {
      if (m_storage[i].m_plugin != nullptr)
      {
        return &m_storage[i];
      }
    }

    return nullptr;
  }

  void PluginManager::Init() {}

  PluginRegister* PluginManager::GetRegister(const String& file)
  {
    for (size_t i = 0; i < m_storage.size(); i++)
    {
      if (m_storage[i].m_file == file)
      {
        return &m_storage[i];
      }
    }

    return nullptr;
  }

  PluginManager::PluginManager() {}

  PluginManager::~PluginManager() {}

} // namespace ToolKit
