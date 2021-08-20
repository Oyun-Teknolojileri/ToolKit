#include "stdafx.h"

#include "App.h"
#include "Types.h"
#include "Mod.h"
#include "UI.h"
#include "DebugNew.h"

#include "ImGui/imgui_impl_sdl.h"
#include "SDL.h"

#include <stdio.h>
#include <chrono>

// #define TK_PROFILE

// Global handles.
namespace ToolKit
{
  namespace Editor
  {
    bool g_running = true;
    SDL_Window* g_window = nullptr;
    SDL_GLContext g_context = nullptr;
    App* g_app;

    // Setup.
    const char* appName = "ToolKit";
    const int width = 1280;
    const int height = 720;
#ifdef TK_PROFILE
    const uint fps = 5000;
#else
    const uint fps = 120;
#endif

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

        g_window = SDL_CreateWindow(appName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
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
            glewExperimental = true;
            GLenum err = glewInit();
            if (GLEW_OK != err)
            {
              g_running = false;
              return;
            }

            Main::GetInstance()->Init();

            // Set defaults
            SDL_GL_SetSwapInterval(0);
            glPointSize(5.0f);

            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);

            // Init app
            g_app = new App(width, height);
            g_app->Init();

            // Init UI
            UI::Init();
          }
        }
      }
    }

    float GetMilliSeconds()
    {
      using namespace std::chrono;

      static high_resolution_clock::time_point t1 = high_resolution_clock::now();
      high_resolution_clock::time_point t2 = high_resolution_clock::now();
      duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

      return (float)(time_span.count() * 1000.0);
    }

    void Exit()
    {
      g_running = false;
      SafeDel(g_app);

      UI::UnInit();
      Main::GetInstance()->Uninit();

      SDL_DestroyWindow(g_window);
      SDL_Quit();
    }

    void ProcessEvent(SDL_Event e)
    {
      ImGui_ImplSDL2_ProcessEvent(&e);

      if (e.type == SDL_WINDOWEVENT)
      {
        if (e.window.event == SDL_WINDOWEVENT_RESIZED)
        {
          g_app->OnResize(e.window.data1, e.window.data2);
        }

        if (e.window.event == SDL_WINDOWEVENT_MAXIMIZED)
        {
          g_app->m_windowMaximized = true;
        }

        if (e.window.event == SDL_WINDOWEVENT_RESTORED)
        {
          g_app->m_windowMaximized = false;
        }
      }

      if (e.type == SDL_DROPFILE)
      {
        UI::ImportData.files.push_back(e.drop.file);
        UI::ImportData.showImportWindow = true;
      }

      if (e.type == SDL_QUIT)
      {
        g_app->OnQuit();
      }

      if (e.type == SDL_KEYDOWN)
      {
        switch (e.key.keysym.sym)
        {
        case SDLK_ESCAPE:
          g_app->OnQuit();
          break;
        default:
          break;
        }
      }
    }

    struct Timing
    {
      Timing()
      {
        lastTime = GetMilliSeconds();
        currentTime = 0.0f;
        deltaTime = 1000.0f / fps;
        frameCount = 0;
        timeAccum = 0.0f;
      }

      float lastTime = 0.0f;
      float currentTime = 0.0f;
      float deltaTime = 0.0f;
      float timeAccum = 0.0f;
      int frameCount = 0;
    };

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
    void Emsc_Loop(void* args)
    {
      Timing* timer = static_cast<Timing*> (args);
      SDL_Event sdlEvent;
      while (SDL_PollEvent(&sdlEvent))
      {
        ProcessEvent(sdlEvent);
      }

      timer->currentTime = GetMilliSeconds();
      if (timer->currentTime > timer->lastTime + timer->deltaTime)
      {
        // Update & Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        g_app->Frame(timer->currentTime - timer->lastTime);
        SDL_GL_SwapWindow(g_window);

        timer->frameCount++;
        timer->timeAccum += timer->currentTime - timer->lastTime;
        if (timer->timeAccum >= 1000.0f)
        {
          g_app->m_fps = timer->frameCount;
          timer->timeAccum = 0;
          timer->frameCount = 0;
        }

        timer->lastTime = timer->currentTime;
      }
    }

    int ToolKit_Main_Emsc(int argc, char* argv[])
    {
      Init();

      SDL_MaximizeWindow(g_window);

      static Timing timer;
      emscripten_set_main_loop_arg(Emsc_Loop, reinterpret_cast<void*> (&timer), 0, 1);

      return 0;
    }
#endif

    int ToolKit_Main(int argc, char* argv[])
    {
#ifndef __clang__
      _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

      Init();

      // Restore window.
      uint appWidth = g_app->m_renderer->m_windowWidth;
      uint appHeight = g_app->m_renderer->m_windowHeight;

      if (appWidth != width || appHeight != height)
      {
        SDL_SetWindowSize(g_window, appWidth, appHeight);
        SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
      }

      if (g_app->m_windowMaximized)
      {
        SDL_MaximizeWindow(g_window);
      }

      // Continue with editor.
      std::shared_ptr<Timing> timer = std::make_shared<Timing>();
      while (g_running)
      {
        SDL_Event sdlEvent;
        while (SDL_PollEvent(&sdlEvent))
        {
          ProcessEvent(sdlEvent);
        }

        timer->currentTime = GetMilliSeconds();
        if (timer->currentTime > timer->lastTime + timer->deltaTime)
        {
          // Update & Render
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          g_app->Frame(timer->currentTime - timer->lastTime);
          SDL_GL_SwapWindow(g_window);

          timer->frameCount++;
          timer->timeAccum += timer->currentTime - timer->lastTime;
          if (timer->timeAccum >= 1000.0f)
          {
            g_app->m_fps = timer->frameCount;
            timer->timeAccum = 0;
            timer->frameCount = 0;
          }

          timer->lastTime = timer->currentTime;
        }
#ifndef TK_PROFILE
        else
        {
          SDL_Delay(10);
        }
#endif
      }

      Exit();

      return 0;
    }

  }
}

int main(int argc, char* argv[])
{
#ifdef __EMSCRIPTEN__
  return ToolKit::Editor::ToolKit_Main_Emsc(argc, argv);
#else
  return ToolKit::Editor::ToolKit_Main(argc, argv);
#endif
}