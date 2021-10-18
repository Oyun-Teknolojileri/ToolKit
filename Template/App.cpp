#include "App.h"
#include "SDL.h"

namespace ToolKit
{

  void Game::Init(Main* master)
  {
    Main::SetProxy(master);
  }

  void Game::Destroy()
  {
  }

  void Game::Frame(float deltaTime, class Viewport* viewport)
  {
  }

  void Game::Event(SDL_Event event)
  {
  }

}