/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "PluginManager.h"

#include "Logger.h"
#include "ToolKit.h"

#define CR_HOST CR_UNSAFE
#include <cr/cr.h>

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

    // Check the loaded plugin.
    if (PluginRegister* reg = GetRegister(fullPath))
    {
      if (reg->m_loaded)
      {
        // hot reload.
        assert(reg->m_context != nullptr);
        cr_plugin* context = static_cast<cr_plugin*>(reg->m_context);
        int res            = cr_plugin_update(*context);
        return res >= 0;
      }
    }

    cr_plugin* context = new cr_plugin();
    if (!cr_plugin_open(*context, fullPath.c_str()))
    {
      SafeDel(context);

      TK_ERR("Can not open plugin file %s", fullPath.c_str());
      return false;
    }

    String pluginPath;
    DecomposePath(file, &pluginPath, nullptr, nullptr);
    String pluginTmp = ConcatPaths({pluginPath, "tmp", ""});

    cr_set_temporary_path(*context, pluginTmp.c_str());

    // Load the plugin.
    if (cr_plugin_update(*context, true) < 0)
    {
      SafeDel(context);

      TK_ERR("Can not load plugin file %s", fullPath.c_str());
      return false;
    }

    // Initialize the plugin.
    if (ModuleHandle handle = static_cast<cr_internal*>(context->p)->handle)
    {
      FunctionAdress ProcAddr = (FunctionAdress) GetFunction(handle, "CreateInstance");

      if (ProcAddr != nullptr)
      {
        PluginRegister reg;
        reg.m_context = context;
        reg.m_module  = handle;
        reg.m_plugin  = (ProcAddr) ();
        reg.m_plugin->Init(Main::GetInstance());
        reg.m_loaded = true;
        reg.m_file   = fullPath;

        m_storage.push_back(reg);
        return true;
      }
      else
      {
        SafeDel(context);
        TK_ERR("Can not find CreateInstance() in the plugin.");
      }
    }

    SafeDel(context);
    return false;
  }

  void PluginManager::UnInit()
  {
    for (auto itr = m_storage.rbegin(); itr != m_storage.rend(); itr++)
    {
      Unload(itr->m_file);
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

    reg->m_plugin->Destroy();
    reg->m_plugin = nullptr;

    if (cr_plugin* context = static_cast<cr_plugin*>(reg->m_context))
    {
      cr_plugin_close(*context);
    }

    m_storage.erase(m_storage.begin() + indx);

    SafeDel(reg->m_context);
    reg->m_loaded = false;
  }

  bool PluginManager::Reload(const String& file) { return false; }

  void PluginManager::Update(float deltaTime)
  {
    for (PluginRegister& reg : m_storage)
    {
      if (cr_plugin* context = static_cast<cr_plugin*>(reg.m_context))
      {
        bool needsUpdate = cr_plugin_changed(*context);
        if (cr_plugin_update(*context) >= 0)
        {
          if (needsUpdate)
          {
            cr_internal* crInt      = static_cast<cr_internal*>(context->p);
            reg.m_module            = crInt->handle;
            FunctionAdress ProcAddr = (FunctionAdress) GetFunction(reg.m_module, "CreateInstance");
            reg.m_plugin            = ProcAddr();

            // Polymorphic acess will fail, because vtable is invalid.
            //GamePlugin* gp          = (GamePlugin*) reg.m_plugin;
            //gp->Frame(0);

            TK_LOG("EDS %p", (void*) &*reg.m_plugin);
          }
        }
        else
        {
          TK_ERR("CR Fatal");
        }
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
