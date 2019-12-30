#include "stdafx.h"

#ifdef ABT_TEST_CASE

#include <SDL.h>
#include <Windows.h>
#include "ABT.h"
#include "TestApp.h"
glm::vec3 g_tv;

bool g_running = true;
SDL_Window* g_window = nullptr;
SDL_GLContext g_context = nullptr;
TestApp* g_app;

void ProcessEvent(SDL_Event e)
{
  if (e.type == SDL_QUIT)
    g_running = false;

  if (e.type == SDL_KEYDOWN)
  {
    switch (e.key.keysym.sym)
    {
    case SDLK_ESCAPE:
      g_running = false;
      break;
    default:
      break;
    }
  }

  static bool lbtnclcked = false;
  if (e.type == SDL_MOUSEBUTTONDOWN)
  {
    if (e.button.button == SDL_BUTTON_LEFT)
      lbtnclcked = true;
  }

  if (e.type == SDL_MOUSEBUTTONUP)
  {
    if (e.button.button == SDL_BUTTON_LEFT)
      lbtnclcked = false;
  }

  if (e.type == SDL_MOUSEMOTION)
  {
    if (lbtnclcked)
    {
      g_app->m_cam.Pitch(-e.motion.yrel / 30.0f);
      g_app->m_cam.RotateOnUpVector(-e.motion.xrel / 30.0f);
    }
  }
}

void Init()
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)
  {
    g_running = false;
  }
  else
  {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    g_window = SDL_CreateWindow("ABT TEST", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (g_window == nullptr)
    {
      g_running = false;
    }
    else
    {
      g_context = SDL_GL_CreateContext(g_window);
      if (g_context = nullptr)
      {
        g_running = false;
      }
      else
      {
        // Init glew
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
          g_running = false;
          return;
        }
        
        // Set defaults
        SDL_GL_SetSwapInterval(1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        // Init app
        g_app = new TestApp();
        g_app->Init();
      }
    }
  }
}

void Exit()
{
  g_running = false;
  SafeDel(g_app);

  SDL_DestroyWindow(g_window);
  SDL_Quit();
}

int main(int argc, char *argv[])
{
  Init();

  SDL_Event e;
  while (g_running)
  {
    while (SDL_PollEvent(&e))
    {
      ProcessEvent(e);
    }

    // Key handlings
    const Uint8 *state = SDL_GetKeyboardState(nullptr);

    if (state[SDL_SCANCODE_W])
      g_app->m_cam.Translate(glm::vec3(0, 0, -0.1));

    if (state[SDL_SCANCODE_S])
      g_app->m_cam.Translate(glm::vec3(0, 0, 0.1));

    if (state[SDL_SCANCODE_D])
      g_app->m_cam.Translate(glm::vec3(0.1, 0, 0));

    if (state[SDL_SCANCODE_A])
      g_app->m_cam.Translate(glm::vec3(-0.1, 0, 0));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    g_app->Frame();
    SDL_GL_SwapWindow(g_window);
  }

  Exit();
  return 0;
}

#endif