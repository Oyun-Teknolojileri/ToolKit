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
    void Event(SDL_Event event);

    // Plugin functions
    ScenePtr GetScene();
    Entity* GetPlayer();

    // Game logic
    void CheckPlayerMove();
    void CheckEnemyMove();
    void CheckPickups();
    void IconAnim(float deltaTime);
    BoundingBox GetForwardBB(Entity* ntt);
    Vec3 GetForwardDir(Entity* ntt);

  public:    
    // Plugin objects.
    ScenePtr m_scene;
    Camera* m_cam = nullptr;
    Viewport* m_viewport = nullptr;
    const float m_blockSize = 2.0f; // Size of the ground blocks.

    // 3 point lighting system.
    Node* m_lightMaster = nullptr;
    LightRawPtrArray m_sceneLights; // { 0:key 1:fill, 2:back }

    bool m_gameOver = false;
  };

}

extern "C" TK_GAME_API ToolKit::Game * __stdcall CreateInstance()
{
  return new ToolKit::Game();
}