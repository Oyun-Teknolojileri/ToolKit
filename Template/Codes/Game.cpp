/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Game.h"

ToolKit::Game Self;

extern "C" TK_PLUGIN_API ToolKit::Game* TK_STDCAL GetInstance() { return &Self; }

namespace ToolKit
{

  Game* g_game = &Self;

  void Game::Init(Main* master) { Main::SetProxy(master); }

  void Game::Destroy() {}

  void Game::Frame(float deltaTime) {}

  void Game::OnLoad(XmlDocumentPtr state) {}

  void Game::OnUnload(XmlDocumentPtr state) {}

} // namespace ToolKit
