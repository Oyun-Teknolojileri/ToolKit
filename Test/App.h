#pragma once

#include "ToolKit.h"
#include "Plugin.h"

namespace ToolKit
{

  class Game : public GamePlugin
  {
  public:
    void Init(Main* master);
    void Destroy();
    void Frame(float deltaTime, Viewport* viewport);
    void Resize(int width, int height);
    void Event(SDL_Event event);

    // Plugin functions
    ScenePtr GetScene();

    // Game logic
    void CheckPlayerMove();
    void IconAnim(float deltaTime);

  public:    
    // Plugin objects.
    ScenePtr m_scene;
    Camera* m_cam = nullptr;
    Viewport* m_viewport = nullptr;

    // 3 point lighting system.
    Node* m_lightMaster = nullptr;
    LightRawPtrArray m_sceneLights; // { 0:key 1:fill, 2:back }
  };

}

extern "C" TK_GAME_API ToolKit::Game * __stdcall GetInstance()
{
  return new ToolKit::Game();
}