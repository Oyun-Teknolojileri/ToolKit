#pragma once

// https://www.codeproject.com/Articles/28969/HowTo-Export-C-classes-from-a-DLL
// https://stackoverflow.com/questions/538134/exporting-functions-from-a-dll-with-dllexport

#ifdef _WIN32
#    ifdef TK_EXPORTS
#        define TK_GAME_API __declspec(dllexport)
#    else
#        define TK_GAME_API __declspec(dllimport)
#    endif
#elif
#    define TK_GAME_API
#endif

union SDL_Event;

namespace ToolKit
{
  class GamePlugin
  {
  public:
    virtual void Init(class Main* master) = 0;
    virtual void Destroy() = 0;
    virtual void Frame(float deltaTime, class Viewport* viewport) = 0;
    virtual void Event(SDL_Event event) = 0;
  };

}
