/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "PluginManager.h"

#include "Logger.h"
#include "ToolKit.h"
#include "Util.h"
#include "ObjectFactory.h"

#include "DebugNew.h"

namespace ToolKit
{

  bool PluginManager::Load(const String& file)
  {
    String fullPath = file;

#ifdef TK_DEBUG
    fullPath += "_d.dll";
#else
    fullPath += ".dll";
#endif

    if (!CheckSystemFile(fullPath))
    {
      TK_ERR("Can not find plugin file %s", fullPath.c_str());
      return false;
    }

    // Check if the dll is changed.
    if (PluginRegister* reg = GetRegister(fullPath))
    {
      if (reg->m_loaded && reg->m_plugin)
      {
        String cTime = GetCreationTime(fullPath);
        if (reg->m_lastWriteTime != cTime)
        {
          TK_LOG("A new version of the plugin has been found.");
          reg->m_stateCache = MakeNewPtr<XmlDocument>();
          Unload(fullPath);
        }
      }

      if (reg->m_loaded && reg->m_plugin)
      {
        TK_LOG("Plugin has already loaded.");
        return true;
      }
    }

    // Load the plugin.
    ModuleHandle handle = LoadModule(fullPath.c_str());

    // Initialize the plugin.
    if (handle != nullptr)
    {
      FunctionAdress ProcAddr = (FunctionAdress) GetFunction(handle, "GetInstance");

      if (ProcAddr != nullptr)
      {
        PluginRegister reg;
        reg.m_module = handle;
        reg.m_plugin = (ProcAddr) ();
        reg.m_plugin->Init(Main::GetInstance());
        reg.m_initialized = true;
        reg.m_plugin->OnLoad(reg.m_stateCache);
        reg.m_loaded        = true;
        reg.m_stateCache    = nullptr;
        reg.m_file          = fullPath;
        reg.m_lastWriteTime = GetCreationTime(fullPath);

        m_storage.push_back(reg);
        return true;
      }
      else
      {
        TK_ERR("Can not find GetInstance() in the plugin.");
      }
    }
    else
    {
      TK_ERR("Can not load the plugin.");
    }

    return false;
  }

  void PluginManager::UnInit()
  {
    for (int i = (int) m_storage.size() - 1; i >= 0; i--)
    {
      Unload(m_storage[i].m_file);
    }
  }

  void PluginManager::Unload(const String& file)
  {
    int indx            = 0;
    PluginRegister* reg = GetRegister(file, &indx);
    if (reg == nullptr || reg->m_plugin == nullptr || !reg->m_loaded)
    {
      return;
    }

    reg->m_plugin->OnUnload(reg->m_stateCache);
    reg->m_plugin->Destroy();
    reg->m_initialized = false;
    reg->m_plugin      = nullptr;

    FreeModule(reg->m_module);
    reg->m_module = nullptr;
    reg->m_loaded = false;

    m_storage.erase(m_storage.begin() + indx);
  }

  void PluginManager::Update(float deltaTime)
  {
    for (PluginRegister& reg : m_storage)
    {
      if (reg.m_plugin && reg.m_loaded)
      {
        if (reg.m_plugin->m_currentState == PluginState::Started)
        {
          if (!reg.m_initialized)
          {
            reg.m_plugin->Init(Main::GetInstance());
            reg.m_initialized = true;
          }
        }
        else if (reg.m_plugin->m_currentState == PluginState::Running)
        {
          if (!reg.m_initialized)
          {
            reg.m_plugin->Init(Main::GetInstance());
            reg.m_initialized = true;
          }

          reg.m_plugin->Frame(deltaTime);
        }
        else if (reg.m_plugin->m_currentState == PluginState::Stop)
        {
          if (reg.m_initialized)
          {
            reg.m_plugin->Destroy();
            reg.m_initialized = false;
          }
        }
        // Else, paused. Do nothing.
      }
    }
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

  PluginRegister* PluginManager::GetRegister(const String& file, int* indx)
  {
    for (size_t i = 0; i < m_storage.size(); i++)
    {
      if (m_storage[i].m_file == file)
      {
        if (indx != nullptr)
        {
          *indx = (int) i;
        }

        return &m_storage[i];
      }
    }

    return nullptr;
  }

  PluginManager::PluginManager() {}

  PluginManager::~PluginManager() {}

} // namespace ToolKit
