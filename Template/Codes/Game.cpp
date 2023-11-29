/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Game.h"

#include <cr/cr.h>

ToolKit::Game CR_STATE Self;

extern "C" TK_PLUGIN_API ToolKit::Game* TK_STDCAL CreateInstance() { return &Self; }

namespace ToolKit
{

#ifndef GAME_BUILD
  extern Game* ToolKit::g_game = &Self;
#endif

  void Game::Init(Main* master) { Main::SetProxy(master); }

  void Game::Destroy() {}

  void Game::Frame(float deltaTime) {}

  void Game::OnLoad() { ToolKit::g_game = this; }

  void Game::OnUnload() { ToolKit::g_game = this; }

} // namespace ToolKit

CR_EXPORT int cr_main(struct cr_plugin* ctx, enum cr_op operation)
{
  if (operation == CR_STEP)
  {
    Self.Frame(0.0f);
  }

  if (operation == CR_CLOSE)
  {
    // Plugin closing.
  }

  if (operation == CR_LOAD)
  {
    TK_LOG("PLS %p", (void*) &Self);
    Self.OnLoad();
  }

  if (operation == CR_UNLOAD)
  {
    TK_LOG("PUS %p", (void*) &Self);
    Self.OnUnload();
  }

  return 0;
}