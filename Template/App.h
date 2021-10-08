#pragma once

#include "ToolKit.h"
#include "Plugin.h"

namespace ToolKit
{

  class Game : public GamePlugin
  {
  public:
    void Init(::ToolKit::Main* master);
    void Destroy();
    void Frame(float deltaTime);
    void Resize(int width, int height);
    void Event(SDL_Event event);

  public:
    Main* m_main = nullptr;
  };

}

extern "C" TK_GAME_API ToolKit::Game* __stdcall CreateInstance()
{
  return new ToolKit::Game();
}