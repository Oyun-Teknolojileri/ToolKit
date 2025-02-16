/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Anchor.h"
#include "AndroidBuildWindow.h"
#include "App.h"
#include "ConsoleWindow.h"
#include "EditorCamera.h"
#include "EditorViewport2d.h"
#include "Gizmo.h"
#include "Grid.h"
#include "Mod.h"
#include "PopupWindows.h"
#include "PreviewViewport.h"
#include "TKStats.h"
#include "UI.h"

#include <Common/SDLEventPool.h>
#include <Common/Win32Utils.h>
#include <FileManager.h>
#include <GlErrorReporter.h>
#include <ImGui/backends/imgui_impl_sdl2.h>
#include <PluginManager.h>
#include <SDL.h>
#include <Types.h>
#include <locale.h>

#include <array>
#include <chrono>

#include <DebugNew.h>

SDL_Window* g_window    = nullptr;
SDL_GLContext g_context = nullptr;

namespace ToolKit
{
  namespace Editor
  {

    bool g_running                            = true;
    App* g_app                                = nullptr;
    Main* g_proxy                             = nullptr;
    SDLEventPool<TK_PLATFORM>* g_sdlEventPool = nullptr;

    // Windows util function for creating ToolKit Cfg files in AppData.
    void CreateAppData()
    {
      // For windows check appdata.
      StringView appData = getenv("APPDATA");
      if (appData.empty())
      {
        return;
      }

      std::array<String, 4> files = {"Workspace.settings", "Editor.settings", "UILayout.ini", "Engine.settings"};
      String cfgPath              = ConcatPaths({String(appData), "ToolKit", "Config"});

      // Create ToolKit Configs.
      bool doesConfigFolderExists = true;
      if (!CheckSystemFile(cfgPath))
      {
        doesConfigFolderExists = std::filesystem::create_directories(cfgPath);
      }
      if (doesConfigFolderExists)
      {
        for (int i = 0; i < files.size(); i++)
        {
          String targetFile = ConcatPaths({cfgPath, files[i]});
          if (!CheckSystemFile(targetFile))
          {
            String sourceFile = ConcatPaths({ConfigPath(), files[i]});
            if (CheckSystemFile(sourceFile))
            {
              std::filesystem::copy(sourceFile, targetFile, std::filesystem::copy_options::overwrite_existing);
            }
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
          utf8Path.erase(remove(utf8Path.begin(), utf8Path.end(), '\"'), utf8Path.end());
          UnixifyPath(utf8Path);

          file << utf8Path;
        }
        file.close();
      }

      Main::GetInstance()->SetConfigPath(cfgPath);
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

    void PreInit()
    {
      g_sdlEventPool = new SDLEventPool<TK_PLATFORM>();

      // PreInit Main
      g_proxy        = new Main();
      Main::SetProxy(g_proxy);
      CreateAppData();
      g_proxy->PreInit();

      // Platform dependent function assignments.
      GetPluginManager()->FreeModule      = &PlatformHelpers::TKFreeModule;
      GetPluginManager()->LoadModule      = &PlatformHelpers::TKLoadModule;
      GetPluginManager()->GetFunction     = &PlatformHelpers::TKGetFunction;
      GetPluginManager()->GetCreationTime = &PlatformHelpers::GetCreationTime;
      GetLogger()->SetPlatformConsoleFn([](LogType type, const String& msg) -> void
                                        { ToolKit::PlatformHelpers::OutputLog((int) type, msg.c_str()); });
    }

    void Init()
    {
      EngineSettings& settings  = GetEngineSettings();
      const String settingsFile = EngineSettingsPath();
      settings.Load(settingsFile);

      // Init SDL
      if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) < 0)
      {
        g_running = false;
      }
      else
      {

#ifdef TK_GL_ES_3_0
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

// Opengl debuging & profiling features requires es 3_2 context
#ifdef TK_GL_ES_3_2
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);

#ifdef TK_DEBUG
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

        g_window =
            SDL_CreateWindow(settings.Window.Name.c_str(),
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             settings.Window.Width,
                             settings.Window.Height,
                             SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

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
            SDL_GL_MakeCurrent(g_window, g_context);

            // Init OpenGl.
            g_proxy->m_renderSys->InitGl(SDL_GL_GetProcAddress,
                                         [](const std::string& msg) -> void
                                         {
                                           if (g_app == nullptr)
                                           {
                                             return;
                                           }

                                           if (g_app->m_showGraphicsApiErrors)
                                           {
                                             TK_ERR(msg.c_str());
                                           }

                                           GetLogger()->WritePlatformConsole(LogType::Error, msg.c_str());
                                         });

            // Init Main.
            // Override SceneManager.
            SafeDel(g_proxy->m_sceneManager);
            g_proxy->m_sceneManager = new EditorSceneManager();
            g_proxy->Init();

            GetFileManager()->m_ignorePakFile = true;

            // Register Custom Classes.
            ObjectFactory* objFactory         = g_proxy->m_objectFactory;
            objFactory->Register<Grid>();
            objFactory->Register<Anchor>();
            objFactory->Register<Cursor>();
            objFactory->Register<Axis3d>();
            objFactory->Register<LinearGizmo>();
            objFactory->Register<MoveGizmo>();
            objFactory->Register<ScaleGizmo>();
            objFactory->Register<PolarGizmo>();
            objFactory->Register<SkyBillboard>();
            objFactory->Register<LightBillboard>();
            objFactory->Register<GridFragmentShader>();

            // Windows.
            objFactory->Register<ConsoleWindow>();
            objFactory->Register<EditorViewport>();
            objFactory->Register<EditorViewport2d>();
            objFactory->Register<PreviewViewport>();
            objFactory->Register<FolderWindow>();
            objFactory->Register<MultiChoiceWindow>();
            objFactory->Register<OutlinerWindow>();
            objFactory->Register<SimulationWindow>();
            objFactory->Register<PropInspectorWindow>();
            objFactory->Register<RenderSettingsWindow>();
            objFactory->Register<StatsWindow>();
            objFactory->Register<StringInputWindow>();
            objFactory->Register<YesNoWindow>();
            objFactory->Register<MaterialWindow>();
            objFactory->Register<AndroidBuildWindow>();
            objFactory->Register<PluginWindow>();
            objFactory->Register<PluginSettingsWindow>();

            // Overrides.
            objFactory->Override<EditorDirectionalLight, DirectionalLight>();
            objFactory->Override<EditorPointLight, PointLight>();
            objFactory->Override<EditorSpotLight, SpotLight>();
            objFactory->Override<EditorScene, Scene>();
            objFactory->Override<EditorCamera, Camera>();

            // Set defaults
            SDL_GL_SetSwapInterval(0);

            // Init app
            g_app                   = new App(settings.Window.Width, settings.Window.Height);
            g_app->m_sysComExecFn   = &ToolKit::PlatformHelpers::SysComExec;
            g_app->m_shellOpenDirFn = &ToolKit::PlatformHelpers::OpenExplorer;

            g_app->Init();

            // Register update functions

            TKUpdateFn preUpdateFn = [](float deltaTime)
            {
              SDL_Event sdlEvent;
              while (SDL_PollEvent(&sdlEvent))
              {
                g_sdlEventPool->PoolEvent(sdlEvent);
                ProcessEvent(sdlEvent);
              }

              g_app->Frame(deltaTime);
            };
            g_proxy->RegisterPreUpdateFunction(preUpdateFn);

            TKUpdateFn postUpdateFn = [](float deltaTime)
            {
              SDL_GL_MakeCurrent(g_window, g_context);
              SDL_GL_SwapWindow(g_window);

              g_sdlEventPool->ClearPool(); // Clear after consumption.

              g_app->m_lastFrameHWRenderPassCount = Stats::GetHWRenderPassCount();
              g_app->m_lastFrameDrawCallCount     = Stats::GetDrawCallCount();
            };
            g_proxy->RegisterPostUpdateFunction(postUpdateFn);

            // Post init the engine after editor is up.
            g_proxy->PostInit();
          }
        }
      }
    }

    void Exit()
    {
      g_proxy->PreUninit();

      SafeDel(g_app);

      g_proxy->Uninit();
      g_proxy->PostUninit();
      SafeDel(g_proxy);

      SafeDel(g_sdlEventPool);
      SDL_DestroyWindow(g_window);
      SDL_Quit();

      g_running = false;
    }

    void TK_Loop()
    {
      while (g_running)
      {
        if (g_proxy->SyncFrameTime())
        {
          g_proxy->FrameBegin();
          g_proxy->FrameUpdate();
          g_proxy->FrameEnd();

          g_app->m_fps = g_proxy->GetCurrentFPS();
        }
      }
    }

    int ToolKit_Main(int argc, char* argv[])
    {
      PreInit();
      Init();

      TK_Loop();

      Exit();
      return 0;
    }

  } // namespace Editor
} // namespace ToolKit

int main(int argc, char* argv[])
{
  setlocale(LC_ALL, ".UTF-8");
  setlocale(LC_NUMERIC, "C");

#ifdef TK_DEBUG
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  return ToolKit::Editor::ToolKit_Main(argc, argv);
}
