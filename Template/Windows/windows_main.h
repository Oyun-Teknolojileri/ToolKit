#include "../Codes/Game.h"
#include "Common/SDLEventPool.h"
#include "EngineSettings.h"
#include "GlErrorReporter.h"
#include "SDL.h"
#include "Scene.h"
#include "ToolKit.h"
#include "Types.h"
#include "UIManager.h"
#include "GameRenderer.h"
#include "Plugin.h"

#include <stdio.h>
#include <chrono>
#include <iostream>

#define TK_PLATFORM PLATFORM::TKWeb

namespace ToolKit
{
  inline void PlatformPreInit(Main* g_proxy)
  {
    g_proxy->m_resourceRoot = ConcatPaths({ ".", "..", "Resources" });
    g_proxy->m_cfgPath = ConcatPaths({ ".", "..", "Config" });

    GetLogger()->SetWriteConsoleFn([](LogType lt, String ms) -> void { std::cout << ms << std::endl; });
  }

  inline void PlatformMainLoop(Game* g_game, bool& g_running, void (*TK_Loop)())
  {
    while (g_game->m_currentState != PluginState::Stop && g_running)
    {
      TK_Loop();
    }
  }
}