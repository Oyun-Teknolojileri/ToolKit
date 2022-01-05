#pragma once

#ifdef _WIN32 // Windows.
  #define TK_GAME_API __declspec(dllexport)
#else // Other OS.
  #define TK_GAME_API
#endif

namespace ToolKit
{

  class GamePlugin
  {
  public:
    virtual void Init(class Main* master) = 0;
    virtual void Destroy() = 0;
    virtual void Frame(float deltaTime, class Viewport* viewport) = 0;
  };

}
