#include "App.h"
#include "SDL.h"

namespace ToolKit
{
  void Game::Init(Main* master)
  {
    m_main = master;
  }

  void Game::Destroy()
  {
  }

  void Game::Frame(float deltaTime)
  {
  }

  void Game::Resize(int width, int height)
  {
  }

  void Game::Event(SDL_Event event)
  {
  }
}