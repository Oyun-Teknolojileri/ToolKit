#include "stdafx.h"
#include "PluginManager.h"
#include "ToolKit.h"
#include "Logger.h"
#include "DebugNew.h"

namespace ToolKit
{
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef near
#undef far

  typedef ToolKit::GamePlugin* (__cdecl* TKPROC)();

  void PluginManager::Load(const String& name)
  {
    HINSTANCE hinstLib;
    TKPROC ProcAdd;
    BOOL fRunTimeLinkSuccess = FALSE;

    String dllName = name;
#ifdef TK_DEBUG
    dllName += "_d.dll";
#else
    dllName += ".dll";
#endif

    hinstLib = LoadLibrary(dllName.c_str());

    if (hinstLib != NULL)
    {
      m_moduleHandle = (void*)hinstLib;
      ProcAdd = (TKPROC)GetProcAddress(hinstLib, "GetInstance");

      if (NULL != ProcAdd)
      {
        fRunTimeLinkSuccess = TRUE;
        m_plugin = (ProcAdd)();
        m_plugin->Init(ToolKit::Main::GetInstance());
      }
    }

    if (!fRunTimeLinkSuccess)
    {
      m_reporterFn("Can not load plugin module " + dllName);
    }
  }

  void PluginManager::Unload()
  {
    if (m_plugin)
    {
      m_plugin->Destroy();
      SafeDel(m_plugin);
    }

    if (m_moduleHandle)
    {
      FreeLibrary((HINSTANCE)m_moduleHandle);
      m_moduleHandle = nullptr;
    }
  }
#else 
  PluginManager::~PluginManager() { }
  void PluginManager::Load(const String& name) { }
  void PluginManager::Unload() { }
#endif

  void PluginManager::Init()
  {
    m_reporterFn = [](const String& msg) -> void
    {
      Logger::GetInstance()->Log(msg);
    };
  }

  void PluginManager::UnInit()
  {
    Unload();
  }

  PluginManager::PluginManager() 
  {
    m_plugin = nullptr;
  }

  PluginManager::~PluginManager() 
  {
    UnInit();
  }

}