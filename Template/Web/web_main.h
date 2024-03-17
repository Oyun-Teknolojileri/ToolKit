/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "../Codes/Game.h"
#include "Logger.h"
#include "Util.h"
#include "emscripten.h"

#include <stdio.h>

#include <chrono>
#include <iostream>

namespace ToolKit
{
  inline void PlatformPreInit(Main* g_proxy)
  {
    g_proxy->m_resourceRoot = ConcatPaths({".", "..", "Resources"});
    g_proxy->m_cfgPath = ConcatPaths({ ".", "..", "Config", "Web" });

    GetLogger()->SetWriteConsoleFn([](LogType lt, String ms) -> void { std::cout << ms << std::endl; });
  }

  inline void PlatformMainLoop(Game* g_game, bool& g_running, void (*TK_Loop)())
  {
    emscripten_set_main_loop_arg((em_arg_callback_func) TK_Loop, nullptr, 0, 1);
  }

  inline void PlatformAdjustEngineSettings(int availableWidth, int availableHeight, EngineSettings* engineSettings)
  {
  }
} // namespace ToolKit