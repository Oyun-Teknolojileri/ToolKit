#include "App.h"
#include "Common/GlErrorReporter.h"
#include "Common/SDLEventPool.h"
#include "Common/Win32Utils.h"
#include "ConsoleWindow.h"
#include "GL/glew.h"
#include "ImGui/imgui_impl_sdl.h"
#include "Mod.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include "Types.h"
#include "UI.h"

#include <stdio.h>

#include <chrono>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    bool g_running          = true;
    SDL_Window* g_window    = nullptr;
    SDL_GLContext g_context = nullptr;
    App* g_app              = nullptr;
    Main* g_proxy           = nullptr;
    EngineSettings g_settings;

    void GlDebugReportInit()
    {
      if (glDebugMessageCallback != NULL)
      {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(&GLDebugMessageCallback, nullptr);
      }

      GlErrorReporter::Report = [](const std::string& msg) -> void {
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
            glDebugMessageControl(
                GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);

            glDebugMessageControl(GL_DEBUG_SOURCE_API,
                                  GL_DEBUG_TYPE_ERROR,
                                  GL_DEBUG_SEVERITY_HIGH,
                                  0,
                                  NULL,
                                  GL_TRUE);
          }
          else if (state >= 2)
          {
            glDebugMessageControl(
                GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
          }
        }

        if (g_app->m_showGraphicsApiErrors)
        {
          g_app->GetConsole()->AddLog(msg, LogType::Error);
        }
      };
    }

    /*
     * Refactor as below.
     *
     * PreInit Main
     * InitSDL
     * Init App
     * Uninit App
     * Uninit Main
     * Uninit SDL
     * PostUninit Main
     */

    // Windows util function for creating ToolKit Cfg files in AppData.
    void CreateAppData()
    {
      // For windows check appdata.
      StringView appData = getenv("APPDATA");
      if (appData.empty())
      {
        return;
      }

      StringArray files = {"Workspace.settings",
                           "Editor.settings",
                           "UILayout.ini",
                           "Engine.settings"};

      String cfgPath    = ConcatPaths({String(appData), "ToolKit", "Config"});
      String targetFile = ConcatPaths({cfgPath, files[0]});

      // Create ToolKit Configs.
      bool doesConfigFolderExists = true;
      if (!CheckSystemFile(cfgPath))
      {
        doesConfigFolderExists = std::filesystem::create_directories(cfgPath);
      }
      if (doesConfigFolderExists)
      {
        for (int i = 0; i < 4; i++)
        {
          if (!CheckSystemFile(ConcatPaths({cfgPath, files[i]})))
          {
            std::filesystem::copy(
                ConcatPaths({ConfigPath(), files[i]}),
                ConcatPaths({cfgPath, files[i]}),
                std::filesystem::copy_options::overwrite_existing);
          }
        }
      }

      // Create Path file.
      String pathFile = ConcatPaths({cfgPath, "Path.txt"});

      std::fstream file;
      file.open(pathFile, std::ios::trunc | std::ios::out);
      if (file.is_open())
      {
        std::filesystem::path path = std::filesystem::current_path();
        if (path.has_parent_path())
        {
          String utf8Path = path.parent_path().u8string();
          utf8Path.erase(remove(utf8Path.begin(), utf8Path.end(), '\"'),
                         utf8Path.end());

          file << utf8Path;
        }
        file.close();
      }

      Main::GetInstance()->SetConfigPath(cfgPath);
    }

    void PreInit()
    {
      // PreInit Main
      g_proxy = new Main();
      Main::SetProxy(g_proxy);
      CreateAppData();
      g_proxy->PreInit();
    }

    void Init()
    {
      g_settings = g_proxy->m_engineSettings;

      // Init SDL
      if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
      {
        g_running = false;
      }
      else
      {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_ES);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 2);

        if (g_settings.Graphics.MSAA > 0)
        {
          SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
          SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,
                              g_settings.Graphics.MSAA);
        }

#ifdef TK_DEBUG
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

        g_window =
            SDL_CreateWindow(g_settings.Window.Name.c_str(),
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             g_settings.Window.Width,
                             g_settings.Window.Height,
                             SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                 SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

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
            GLenum err       = glewInit();
            if (GLEW_OK != err)
            {
              g_running = false;
              return;
            }

#ifdef TK_DEBUG
            GlDebugReportInit();
#endif

            // Init Main
            // Override SceneManager.
            SafeDel(g_proxy->m_sceneManager);
            g_proxy->m_sceneManager = new EditorSceneManager();
            g_proxy->Init();

            // Set defaults
            SDL_GL_SetSwapInterval(0);

            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);

            // Init app
            g_app = new App(g_settings.Window.Width, g_settings.Window.Height);
            g_app->m_sysComExecFn = ToolKit::Win32Helpers::g_SysComExecFn;
            UI::Init();
            g_app->Init();
          }
        }
      }
    }

    void Exit()
    {
      UI::UnInit();
      SafeDel(g_app);

      g_proxy->Uninit();
      g_proxy->PostUninit();
      SafeDel(g_proxy);

      SDL_DestroyWindow(g_window);
      SDL_Quit();

      g_running = false;
    }

    void ProcessEvent(const SDL_Event& e)
    {
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
        g_app->ManageDropfile(e.drop.file);
      }

      if (e.type == SDL_QUIT)
      {
        g_app->OnQuit();
      }

      ImGui_ImplSDL2_ProcessEvent(&e);
    }

    struct Timing
    {
      explicit Timing(uint fps)
      {
        lastTime    = GetElapsedMilliSeconds();
        currentTime = 0.0f;
        deltaTime   = 1000.0f / static_cast<float>(fps);
        frameCount  = 0;
        timeAccum   = 0.0f;
      }

      float lastTime    = 0.0f;
      float currentTime = 0.0f;
      float deltaTime   = 0.0f;
      float timeAccum   = 0.0f;
      int frameCount    = 0;
    };

    void TK_Loop(void* args)
    {
      Timing* timer = static_cast<Timing*>(args);

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
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                  GL_STENCIL_BUFFER_BIT);

          g_app->Frame(timer->currentTime - timer->lastTime);
          ClearPool(); // Clear after consumption.
          SDL_GL_SwapWindow(g_window);

          timer->frameCount++;
          timer->timeAccum += timer->currentTime - timer->lastTime;
          if (timer->timeAccum >= 1000.0f)
          {
            g_app->m_fps      = timer->frameCount;
            timer->timeAccum  = 0;
            timer->frameCount = 0;
          }

          timer->lastTime = timer->currentTime;
        }
      }
    }

    int ToolKit_Main(int argc, char* argv[])
    {
      PreInit();
      Init();

      static Timing timer(g_settings.Graphics.FPS);
      TK_Loop(reinterpret_cast<void*>(&timer));

      Exit();
      return 0;
    }

  } // namespace Editor
} // namespace ToolKit

int main(int argc, char* argv[])
{
#ifdef TK_DEBUG
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  return ToolKit::Editor::ToolKit_Main(argc, argv);
}
