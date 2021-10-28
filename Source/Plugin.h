#pragma once

namespace ToolKit
{

  #ifdef _WIN32
    #define TK_GAME_API __declspec(dllexport)
    #define TK_STDCAL __stdcall
  #else
    #define TK_GAME_API
    #define TK_STDCAL
  #endif

  class GamePlugin
  {
  public:
    virtual void Init(class Main* master) = 0;
    virtual void Destroy() = 0;
    virtual void Frame(float deltaTime, class Viewport* viewport) = 0;
  };

}
