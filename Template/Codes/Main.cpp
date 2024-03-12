#include "TKPlatform.h"

#ifdef TK_WIN
#include "windows_main.h"
#endif
#ifdef TK_WEB
#include "web_main.h"
#endif
#ifdef TK_ANDROID
#include "android_main.h"
#endif

namespace ToolKit
{
  Game* g_game = nullptr;
  bool g_running = true;
  SDL_Window* g_window = nullptr;
  SDL_GLContext g_context = nullptr;
  Main* g_proxy = nullptr;
  Viewport* g_viewport = nullptr;
  EngineSettings* g_engineSettings = nullptr;
  SDLEventPool<TK_PLATFORM>* g_sdlEventPool = nullptr;
  GameRenderer* g_gameRenderer = nullptr;

  // Setup.
  const char* g_appName = "ToolKit";
  const uint g_targetFps = 120;

  void ProcessEvent(const SDL_Event& e)
  {
    if (e.type == SDL_QUIT)
    {
      g_running = false;
    }
  }

  void PreInit()
  {
    g_sdlEventPool = new SDLEventPool<TK_PLATFORM>();

    // PreInit Main
    g_proxy = new Main();
    Main::SetProxy(g_proxy);

    g_proxy->PreInit();

    PlatformPreInit(g_proxy);
  }

  void Init()
  {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) < 0)
    {
      g_running = false;
    }
    else
    {
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

      // EGL does not support sRGB backbuffer. Need to use an extension
      // https://stackoverflow.com/questions/20396523/android-egl-srgb-default-renderbuffer

      SDL_DisplayMode DM;
      SDL_GetCurrentDisplayMode(0, &DM);
      g_proxy->m_engineSettings->Window.Width = DM.w;
      g_proxy->m_engineSettings->Window.Height = DM.h;
      g_engineSettings = g_proxy->m_engineSettings;

      g_window =
        SDL_CreateWindow(g_appName,
          SDL_WINDOWPOS_UNDEFINED,
          SDL_WINDOWPOS_UNDEFINED,
          g_engineSettings->Window.Width,
          g_engineSettings->Window.Height,
          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

      if (g_window == nullptr)
      {
        g_running = false;
      }
      else
      {
        g_context = SDL_GL_CreateContext(g_window);
        if (g_context == nullptr)
        {
          const char* error = SDL_GetError();
          TK_LOG("%s", error);
          g_running = false;
        }
        else
        {
          SDL_GL_MakeCurrent(g_window, g_context);

          const char* error = SDL_GetError();
          TK_LOG("%s", error);

          // Init OpenGl.
          g_proxy->m_renderSys->InitGl((void*)SDL_GL_GetProcAddress, [](const String& msg) { TK_LOG("%s", msg.c_str()); });

          // Set defaults
          SDL_GL_SetSwapInterval(0);

          // ToolKit Init
          g_proxy->Init();

          // Init viewport and window size
          g_viewport =
            new GameViewport((float)g_engineSettings->Window.Width, (float)g_engineSettings->Window.Height);
          GetUIManager()->RegisterViewportToUpdateLayers(g_viewport);
          GetRenderSystem()->SetAppWindowSize(g_engineSettings->Window.Width, g_engineSettings->Window.Height);

          // Init game
          g_game = new Game();
          g_game->Init(g_proxy);
          g_game->SetViewport(g_viewport);
          g_game->m_currentState = PluginState::Running;

          g_gameRenderer = new GameRenderer();

          g_game->OnPlay();

          // Register update functions

          TKUpdateFn preUpdateFn = [](float deltaTime)
            {
              SDL_Event sdlEvent;
              while (SDL_PollEvent(&sdlEvent))
              {
                g_sdlEventPool->PoolEvent(sdlEvent);
                ProcessEvent(sdlEvent);
              }
            };
          g_proxy->RegisterPreUpdateFunction(preUpdateFn);

          TKUpdateFn postUpdateFn = [](float deltaTime)
            {
              g_viewport->Update(deltaTime);
              g_game->Frame(deltaTime);

              GameRendererParams params;
              params.gfx = g_engineSettings->PostProcessing;
              params.scene = GetSceneManager()->GetCurrentScene();
              params.useMobileRenderPath = true;
              params.viewport = g_viewport;
              g_gameRenderer->SetParams(params);

              GetRenderSystem()->AddRenderTask({ [deltaTime](Renderer* renderer) -> void
                                                {
                  g_gameRenderer->Render(renderer);
                } });

              SDL_GL_SwapWindow(g_window);

              g_sdlEventPool->ClearPool(); // Clear after consumption.
            };
          g_proxy->RegisterPostUpdateFunction(postUpdateFn);
        }
      }
    }
  }

  float GetMilliSeconds()
  {
    static std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

    return (float)time_span.count() * 1000.0f;
  }

  void Exit()
  {
    SafeDel(g_gameRenderer);

    g_game->Destroy();
    Main::GetInstance()->Uninit();
    SafeDel(g_proxy);

    SafeDel(g_sdlEventPool);
    SDL_DestroyWindow(g_window);
    SDL_Quit();

    g_running = false;
  }

  void TK_Loop()
  {
    if (g_proxy->SyncFrameTime())
    {
      g_proxy->FrameBegin();
      g_proxy->FrameUpdate();
      g_proxy->FrameEnd();
    }
  }

  int ToolKit_Main(int argc, char* argv[])
  {
    PreInit();
    Init();

    PlatformMainLoop(g_game, g_running, TK_Loop);

    Exit();
    return 0;
  }

} // namespace ToolKit

int main(int argc, char* argv[]) { return ToolKit::ToolKit_Main(argc, argv); }