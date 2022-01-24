#include "Game.h"

extern "C" TK_GAME_API ToolKit::Game * TK_STDCAL CreateInstance()
{
  return new ToolKit::Game();
}

namespace ToolKit
{

  void Game::Init(Main* master)
  {
    Main::SetProxy(master);
  }

  void Game::Destroy()
  {
    delete this;
  }

  void Game::Frame(float deltaTime, class Viewport* viewport)
  {
  }

}