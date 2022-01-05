#include "stdafx.h"
#include "PluginManager.h"
#include "ToolKit.h"
#include "Logger.h"
#include "DebugNew.h"

namespace ToolKit
{

  typedef ToolKit::GamePlugin* (__cdecl* TKPROC)();

#ifdef _WIN32
  #define NOMINMAX
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
  #include <fileapi.h>
  
  #undef near
  #undef far

  bool PluginManager::Load(const String& file)
  {
    HINSTANCE hinstLib;
    TKPROC ProcAdd;

    String dllName = file;
#ifdef TK_DEBUG
    dllName += "_d.dll";
#else
    dllName += ".dll";
#endif

    // Check the register.
    bool newRegAdded = false;
    PluginRegister* reg = nullptr;
    if (reg = GetRegister(dllName))
    {
      // Unload if dirty
      if (reg->m_loaded)
      {
        // Get file creation time.  
        WIN32_FILE_ATTRIBUTE_DATA attrData;
        GetFileAttributesEx(dllName.c_str(), GetFileExInfoStandard, &attrData);
        String created = std::to_string(attrData.ftLastWriteTime.dwHighDateTime) +
          std::to_string(attrData.ftLastWriteTime.dwLowDateTime);

        if (reg->m_lastWriteTime == created)
        {
          m_reporterFn("Plugin is already loaded and up-to-date.");
          return false;
        }
        else
        {
          // Reload.
          Unload(reg->m_file);
          if (reg->m_module)
          {
            FreeLibrary((HINSTANCE)reg->m_module);
          }
        }

        reg->m_lastWriteTime = created;
      }
    }
    else
    {
      // A new reg is needed.
      newRegAdded = true;
      m_storage.push_back(PluginRegister());
      reg = &m_storage.back();
    }

    // SDL sends utf8 paths. You'll eventually find your self here.
    // https://discourse.libsdl.org/t/file-path-and-unicode-characters-sdl-dropfile-event/24764/4
    hinstLib = LoadLibrary(dllName.c_str());

    if (hinstLib != NULL)
    {
      reg->m_module = (void*)hinstLib;
      ProcAdd = (TKPROC)GetProcAddress(hinstLib, "CreateInstance");

      if (ProcAdd != NULL)
      {
        reg->m_plugin = (ProcAdd)();
        reg->m_plugin->Init(Main::GetInstance());
        reg->m_loaded = true;
        reg->m_file = dllName;
        return true;
      }
      else 
      {
        m_reporterFn("Can not find CreateInstance() in the plugin.");
      }
    }
    else
    {
      m_reporterFn("Can not load plugin " + dllName);
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
      if (reg.m_module)
      {
        FreeLibrary((HINSTANCE)reg.m_module);
      }
      reg.m_module = nullptr;
    }

    m_storage.clear();
  }

#else 
  bool PluginManager::Load(const String& name) { return false; }
  void PluginManager::UnInit() {}
#endif

  void PluginManager::Unload(const String& file)
  {
    PluginRegister* reg = GetRegister(file);
    if 
    (
      reg == nullptr 
      || reg->m_plugin == nullptr 
      || !reg->m_loaded
    )
    {
      return;
    }

    /*
    * Just destroy the plugin.
    * Sinces may create objects, upon free,
    * they can leave dangling pointers.
    * To be on the safe side, unload every module on exit.
    */
    reg->m_plugin->Destroy();
    reg->m_loaded = false;
    reg->m_plugin = nullptr;
  }

  void PluginManager::Report(const char* msg, ...)
  {
    va_list args;
    va_start(args, msg);

    static char buff[2048];
    vsprintf(buff, msg, args);
    m_reporterFn(msg);

    va_end(args);
  }

  GamePlugin* PluginManager::GetGamePlugin()
  {
    if (PluginRegister* reg = GetGameRegister())
    {
      return static_cast<GamePlugin*> (reg->m_plugin);
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

  void PluginManager::Init()
  {
    m_reporterFn = [](const String& msg) -> void
    {
      GetLogger()->Log(msg);
    };
  }

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

  PluginManager::PluginManager() 
  {
  }

  PluginManager::~PluginManager() 
  {
  }

}