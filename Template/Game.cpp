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
#ifdef __EMSCRIPTEN__
    GetRenderSystem()->ExecuteRenderTasks();

    m_sceneRenderer.m_params.Cam = camera;
    m_sceneRenderer.m_params.ClearFramebuffer = true;
    m_sceneRenderer.m_params.Gfx.BloomIntensity = 0.0;
    m_sceneRenderer.m_params.Lights = GetSceneManager()->GetCurrentScene()->GetLights();
    m_sceneRenderer.m_params.MainFramebuffer = viewport->m_framebuffer;
    m_sceneRenderer.m_params.Scene = GetSceneManager()->GetCurrentScene();
    GetRenderSystem()->AddRenderTask(&m_sceneRenderer);

    static uint totalFrameCount = 0;
    GetRenderSystem()->SetFrameCount(totalFrameCount++);
#endif
  }

}