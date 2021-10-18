#pragma once

#include "ToolKit.h"
#include "Plugin.h"

namespace ToolKit
{

  class Game : public GamePlugin
  {
  public:
    virtual void Init(class Main* master);
    virtual void Destroy();
    virtual void Frame(float deltaTime, class Viewport* viewport);
    virtual void Event(SDL_Event event);
  };

}

extern "C" TK_GAME_API ToolKit::Game* __stdcall CreateInstance()
{
  return new ToolKit::Game();
}