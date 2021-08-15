#include "SDL.h"
#include "ToolKit.h"
#include "App.h"
#include "SDL_ttf.h"
#include "Audio.h"
#include "GL/glew.h"
#include "DebugNew.h"
#include <ctime>
#include <ratio>
#include <chrono>

bool g_running = true;
SDL_Window* g_window = nullptr;
SDL_GLContext g_context = nullptr;
App* g_app;
const int width = (int)(640 * 2.0f / 3.0f);
const int height = (int)(768 * 2.0f / 3.0f);

void ProcessEvent(SDL_Event e)
{
  if (e.type == SDL_QUIT)
  {
    g_running = false;
  }

  if (e.type == SDL_KEYDOWN)
  {
    switch (e.key.keysym.sym)
    {
    case SDLK_ESCAPE:
      g_running = false;
      break;
    case SDLK_r:
      if (g_app->m_shipGone)
      {
        g_app->m_restartSignaled = true;
      }
      break;
    default:
      break;
    }
  }

  static bool lbtnclcked = false;
  if (e.type == SDL_MOUSEBUTTONDOWN)
  {
    if (e.button.button == SDL_BUTTON_LEFT)
    {
      lbtnclcked = true;
    }
  }

  if (e.type == SDL_MOUSEBUTTONUP)
  {
    if (e.button.button == SDL_BUTTON_LEFT)
    {
      lbtnclcked = false;
    }
  }

  if (e.type == SDL_MOUSEMOTION)
  {
    static bool skip = true;
    if (!skip)
    {
      glm::vec3 pos = g_app->m_crosshair->m_node->GetTranslation();
      g_app->m_sscp += glm::ivec2(e.motion.xrel, -e.motion.yrel);

      if (g_app->m_sscp.x >= width)
      {
        g_app->m_sscp.x = width;
      }

      if (g_app->m_sscp.x < 0)
      {
        g_app->m_sscp.x = 0;
      }

      if (g_app->m_sscp.y >= height)
      {
        g_app->m_sscp.y = height;
      }

      if (pos.y < 0)
      {
        g_app->m_sscp.y = 0;
      }
    }
    skip = false;

    if (lbtnclcked)
    {
      g_app->m_cam.Pitch(-e.motion.yrel / 30.0f);
      g_app->m_cam.RotateOnUpVector(-e.motion.xrel / 30.0f);
    }
  }
}

void Init()
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
  {
    g_running = false;
  }
  else
  {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    g_window = SDL_CreateWindow("YET ANOTHER SPACE SHOOTER", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (g_window == nullptr)
    {
      g_running = false;
    }
    else
    {
      g_context = SDL_GL_CreateContext(g_window);
      if (g_context == nullptr)
      {
        g_running = false;
      }
      else
      {
        // Init glew
        //glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
          g_running = false;
          return;
        }

        if (TTF_Init() == -1)
          return;

        Main::GetInstance()->Init();

        // Set defaults
        SDL_GL_SetSwapInterval(1);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);

        // Init app
        g_app = new App();
        g_app->m_windowWidth = width;
        g_app->m_windowHeight = height;
        g_app->Init();
      }
    }
  }
}

void Exit()
{
  g_running = false;
  Main::GetInstance()->Uninit();
  SafeDel(g_app);

  SDL_DestroyWindow(g_window);
  TTF_Quit();
  SDL_Quit();
}

unsigned long GetMilliSeconds()
{
  using namespace std::chrono;

  static high_resolution_clock::time_point t1 = high_resolution_clock::now();
  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

  return (unsigned long)(time_span.count() * 1000.0);
}

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

uint lastTime = 0, currentTime = 0;
uint fps = 1000 / 60;

void AppLoop()
{
  SDL_Event e;
  while (SDL_PollEvent(&e))
  {
    ProcessEvent(e);
  }

  currentTime = GetMilliSeconds();
  if (currentTime > lastTime + fps)
  {
    // Key handlings
    const Uint8* state = SDL_GetKeyboardState(nullptr);
    if (state[SDL_SCANCODE_W])
      g_app->m_cam.Translate(glm::vec3(0, 0, -0.1));

    if (state[SDL_SCANCODE_S])
      g_app->m_cam.Translate(glm::vec3(0, 0, 0.1));

    if (state[SDL_SCANCODE_D])
      g_app->m_cam.Translate(glm::vec3(0.1, 0, 0));

    if (state[SDL_SCANCODE_A])
      g_app->m_cam.Translate(glm::vec3(-0.1, 0, 0));

    // Auto fire
    int fireRate = 1000 / g_app->m_spaceShip->m_fireRate;
    {
      static int lastFireTime = 0;
      if (currentTime > (uint)(lastFireTime + fireRate))
      {
        for (auto entry : g_app->m_spaceShip->m_fireLocs)
          g_app->m_projectileManager.FireProjectile(entry->GetTranslation(TransformationSpace::TS_WORLD));
        lastFireTime = currentTime;
        if (!g_app->m_shipGone)
          AudioPlayer::Play(&g_app->m_lazerShotSource);
      }
    }

    // Render
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    g_app->Frame(currentTime - lastTime);
    SDL_GL_SwapWindow(g_window);

    lastTime = currentTime;
  }
}
#else
void AppLoop()
{
  SDL_Event e;
  uint lastTime = GetMilliSeconds();
  uint currentTime;
  uint fps = 1000 / 60;
  while (g_running)
  {
    while (SDL_PollEvent(&e))
    {
      ProcessEvent(e);
    }

    currentTime = GetMilliSeconds();
    if (currentTime > lastTime + fps)
    {
      // Key handlings
      const Uint8* state = SDL_GetKeyboardState(nullptr);
      if (state[SDL_SCANCODE_W])
        g_app->m_cam.Translate(glm::vec3(0, 0, -0.1));

      if (state[SDL_SCANCODE_S])
        g_app->m_cam.Translate(glm::vec3(0, 0, 0.1));

      if (state[SDL_SCANCODE_D])
        g_app->m_cam.Translate(glm::vec3(0.1, 0, 0));

      if (state[SDL_SCANCODE_A])
        g_app->m_cam.Translate(glm::vec3(-0.1, 0, 0));

      // Auto fire
      int fireRate = 1000 / g_app->m_spaceShip->m_fireRate;
      {
        static int lastFireTime = 0;
        if (currentTime > (uint)(lastFireTime + fireRate))
        {
          for (auto entry : g_app->m_spaceShip->m_fireLocs)
            g_app->m_projectileManager.FireProjectile(entry->GetTranslation(TransformationSpace::TS_WORLD));
          lastFireTime = currentTime;
          if (!g_app->m_shipGone)
            AudioPlayer::Play(&g_app->m_lazerShotSource);
        }
      }

      // Render
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      g_app->Frame(currentTime - lastTime);
      SDL_GL_SwapWindow(g_window);

      lastTime = currentTime;
    }
    else
    {
      SDL_Delay(10);
    }
  }
}
#endif

int main(int argc, char* argv[])
{
#ifndef __clang__
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  Init();

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(AppLoop, 0, 1);
#else
  AppLoop();
  Exit();
#endif

  return 0;
}
