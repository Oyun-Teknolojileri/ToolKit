#include "App.h"

#include <stdio.h>
#include <chrono>

#include "Types.h"
#include "Mod.h"
#include "UI.h"
#include "ConsoleWindow.h"
#include "Common/GlErrorReporter.h"
#include "Common/SDLEventPool.h"
#include "ImGui/imgui_impl_sdl.h"
#include "GL/glew.h"

#include "SDL.h"
#include "SDL_opengl.h"

#include "DebugNew.h"

namespace ToolKit
{

  namespace Editor
  {

    bool g_running = true;
    SDL_Window* g_window = nullptr;
    SDL_GLContext g_context = nullptr;
    App* g_app = nullptr;
    Main* g_proxy = nullptr;

    // Setup.
    const char* appName = "ToolKit";
    const int width = 1280;
    const int height = 720;
    const uint fps = 120;

    void GlDebugReportInit()
    {
      if (glDebugMessageCallback != NULL)
      {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(&GLDebugMessageCallback, nullptr);
      }

      GlErrorReporter::Report = [](const std::string& msg) -> void
      {
        static byte state = g_app->m_showGraphicsApiErrors;

        if (g_app == nullptr)
        {
          return;
        }

        if (state != g_app->m_showGraphicsApiErrors)
        {
          state = g_app->m_showGraphicsApiErrors;

          if (state == 1)
          {
            glDebugMessageControl
            (
              GL_DONT_CARE,
              GL_DONT_CARE,
              GL_DONT_CARE,
              0,
              NULL, GL_FALSE
            );

            glDebugMessageControl
            (
              GL_DEBUG_SOURCE_API,
              GL_DEBUG_TYPE_ERROR,
              GL_DEBUG_SEVERITY_HIGH,
              0, NULL, GL_TRUE
            );
          }
          else if (state >= 2)
          {
            glDebugMessageControl
            (
              GL_DONT_CARE,
              GL_DONT_CARE,
              GL_DONT_CARE,
              0,
              NULL,
              GL_TRUE
            );
          }
        }

        if (g_app->m_showGraphicsApiErrors)
        {
          g_app->GetConsole()->AddLog(msg, LogType::Error);
        }
      };
    }

    void Init()
    {
      if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
      {
        g_running = false;
      }
      else
      {
        SDL_GL_SetAttribute
        (
          SDL_GL_CONTEXT_PROFILE_MASK,
          SDL_GL_CONTEXT_PROFILE_CORE
        );

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#ifdef TK_DEBUG
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

        g_window = SDL_CreateWindow
        (
          appName,
          SDL_WINDOWPOS_UNDEFINED,
          SDL_WINDOWPOS_UNDEFINED,
          width,
          height,
          SDL_WINDOW_OPENGL |
          SDL_WINDOW_RESIZABLE |
          SDL_WINDOW_SHOWN |
          SDL_WINDOW_ALLOW_HIGHDPI
        );

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
            //  Init glew
            glewExperimental = true;
            GLenum err = glewInit();
            if (GLEW_OK != err)
            {
              g_running = false;
              return;
            }

#ifdef TK_DEBUG
            GlDebugReportInit();
#endif
            g_proxy = new Main();

            // Override SceneManager.
            SafeDel(g_proxy->m_sceneManager);
            g_proxy->m_sceneManager = new EditorSceneManager();

            Main::SetProxy(g_proxy);
            Main::GetInstance()->Init();
            UI::Init();

            // Set defaults
            SDL_GL_SetSwapInterval(0);
            glPointSize(5.0f);

            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);

            // Init app
            g_app = new App(width, height);
            g_app->Init();
          }
        }
      }
    }

    void Exit()
    {
      UI::UnInit();
      SafeDel(g_app);
      Main::GetInstance()->Uninit();
      SafeDel(g_proxy);

      SDL_DestroyWindow(g_window);
      SDL_Quit();

      g_running = false;
    }

    void ProcessEvent(const SDL_Event& e)
    {
      //  If message doesn't meant to be processed in imgui, set this to true.
      bool skipImgui = false;

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
        case SDLK_F5:
          if
          (
            g_app->m_gameMod == GameMod::Playing ||
            g_app->m_gameMod == GameMod::Paused
          )
          {
            g_app->SetGameMod(GameMod::Stop);
          }
          else
          {
            g_app->SetGameMod(GameMod::Playing);
          }
          break;
        case SDLK_ESCAPE:
          if
          (
            g_app->m_gameMod != GameMod::Playing &&
            g_app->m_gameMod != GameMod::Paused
          )
          {
            g_app->OnQuit();
          }
          break;
        case SDLK_s:
          if (SDL_GetModState() & KMOD_LCTRL)
          {
            g_app->OnSaveScene();
            skipImgui = true;
          }
          break;
        default:
          break;
        }
      }

      if (!skipImgui)
      {
        ImGui_ImplSDL2_ProcessEvent(&e);
      }
    }

    struct Timing
    {
      Timing()
      {
        lastTime = GetElapsedMilliSeconds();
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

    void TK_Loop(void* args)
    {
      Timing* timer = static_cast<Timing*> (args);

      while (g_running)
      {
        SDL_Event sdlEvent;
        while (SDL_PollEvent(&sdlEvent))
        {
          PoolEvent(sdlEvent);
          ProcessEvent(sdlEvent);
        }

        timer->currentTime = GetElapsedMilliSeconds();
        if (timer->currentTime > timer->lastTime + timer->deltaTime)
        {
          glClear
          (
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT
          );

          g_app->Frame(timer->currentTime - timer->lastTime);
          ClearPool();  // Clear after consumption.
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
    }

    int ToolKit_Main(int argc, char* argv[])
    {
      Init();

      static Timing timer;
      TK_Loop(reinterpret_cast<void*> (&timer));

      Exit();
      return 0;
    }

  }  // namespace Editor
}  // namespace ToolKit

int main(int argc, char* argv[])
{
#ifdef TK_DEBUG
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  return ToolKit::Editor::ToolKit_Main(argc, argv);
}
