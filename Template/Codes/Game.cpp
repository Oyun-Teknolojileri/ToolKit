/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Game.h"

extern "C" TK_GAME_API ToolKit::Game* TK_STDCAL CreateInstance() { return new ToolKit::Game(); }

namespace ToolKit
{

#ifndef GAME_BUILD
  extern Game* ToolKit::g_game = nullptr;
#endif

  void Game::Init(Main* master)
  {
    Main::SetProxy(master);

    g_game = this;
  }

  void Game::Destroy() { delete this; }

  void Game::Frame(float deltaTime, Viewport* viewport) {}
} // namespace ToolKit