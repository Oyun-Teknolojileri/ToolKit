/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "../Codes/Game.h"
#include "Logger.h"
#include "Util.h"

#include <stdio.h>

#include <chrono>
#include <iostream>

#define PLATFORM_SDL_FLAGS (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI)

namespace ToolKit
{
  inline void PlatformPreInit(Main* g_proxy)
  {
    g_proxy->m_resourceRoot = ConcatPaths({".", "..", "Resources"});
    g_proxy->m_cfgPath      = ConcatPaths({".", "..", "Config"});

    GetLogger()->SetWriteConsoleFn([](LogType lt, String ms) -> void { std::cout << ms << std::endl; });
  }

  inline void PlatformMainLoop(Game* g_game, bool& g_running, void (*TK_Loop)())
  {
    while (g_game->m_currentState != PluginState::Stop && g_running)
    {
      TK_Loop();
    }
  }

  inline void PlatformAdjustEngineSettings(int availableWidth, int availableHeight, EngineSettings* engineSettings)
  {
  }
} // namespace ToolKit