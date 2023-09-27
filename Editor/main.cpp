/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Anchor.h"
#include "App.h"
#include "ConsoleWindow.h"
#include "EditorCamera.h"
#include "Gizmo.h"
#include "Grid.h"
#include "Mod.h"
#include "UI.h"

#include <Common/SDLEventPool.h>
#include <Common/Win32Utils.h>
#include <GlErrorReporter.h>
#include <Meta.h>
#include <SDL.h>
#include <Types.h>
#include <locale.h>

#include <array>
#include <chrono>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {

    bool g_running          = true;
    SDL_Window* g_window    = nullptr;
    SDL_GLContext g_context = nullptr;
    App* g_app              = nullptr;
    Main* g_proxy           = nullptr;

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
            std::filesystem::copy(ConcatPaths({ConfigPath(), files[i]}),
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
          utf8Path.erase(remove(utf8Path.begin(), utf8Path.end(), '\"'), utf8Path.end());
          UnixifyPath(utf8Path);

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
      EngineSettings& settings = GetEngineSettings();

      // Init SDL
      if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) < 0)
      {
        g_running = false;
      }
      else
      {
#ifdef TK_GL_CORE_3_2
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#elif defined(TK_GL_ES_3_0)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif // TK_GL_CORE_3_2

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);

        // SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 16);
        // SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 16);
        // SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 16);
        // SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 16);

        if (settings.Graphics.MSAA > 0)
        {
          SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
          SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, settings.Graphics.MSAA);
        }

#ifdef TK_DEBUG
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

        g_window =
            SDL_CreateWindow(settings.Window.Name.c_str(),
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             settings.Window.Width,
                             settings.Window.Height,
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
            g_running = false;
          }
          else
          {
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
                                             GetLogger()->WriteConsole(LogType::Error, msg.c_str());
                                             GetLogger()->WritePlatformConsole(LogType::Error, msg.c_str());
                                           }
                                         });

            // Init Main.
            // Override SceneManager.
            SafeDel(g_proxy->m_sceneManager);
            g_proxy->m_sceneManager = new EditorSceneManager();
            g_proxy->Init();

            // Register Custom Classes.
            TKObjectFactory* of = g_proxy->m_objectFactory;
            of->Register<Grid>();
            of->Register<Anchor>();
            of->Register<Cursor>();
            of->Register<Axis3d>();
            of->Register<LinearGizmo>();
            of->Register<MoveGizmo>();
            of->Register<ScaleGizmo>();
            of->Register<PolarGizmo>();
            of->Register<SkyBillboard>();
            of->Register<LightBillboard>();
            of->Register<GridFragmentShader>();

            // Overrides.
            of->Override<EditorDirectionalLight, DirectionalLight>();
            of->Override<EditorPointLight, PointLight>();
            of->Override<EditorSpotLight, SpotLight>();
            of->Override<EditorScene, Scene>();
            of->Override<EditorCamera, Camera>();

            // Set defaults
            SDL_GL_SetSwapInterval(0);

            // Init app
            g_app                   = new App(settings.Window.Width, settings.Window.Height);
            g_app->m_sysComExecFn   = &ToolKit::Win32Helpers::SysComExec;
            g_app->m_shellOpenDirFn = &ToolKit::Win32Helpers::OpenExplorer;

            GetLogger()->SetPlatformConsoleFn([](LogType type, const String& msg) -> void
                                              { ToolKit::Win32Helpers::OutputLog((int) type, msg.c_str()); });

            // Allow classes with the MenuMetaKey to be created from the add menu.
            of->m_metaProcessorMap[MenuMetaKey] = [](StringView val) -> void
            {
              g_app->m_customObjectMetaValues.push_back(String(val));
              g_app->ReconstructDynamicMenus();
            };

            // This code just creates a dummy Primiatives menu to demonstrate the feature.
            // Game plugins should extend the editor with their custom types this way.
            g_app->m_customObjectMetaValues.push_back("Primatives/Helper/Arrow2d:Arrow");
            g_app->m_customObjectMetaValues.push_back("Primatives/Geometry/Cube:Cube");
            g_app->m_customObjectMetaValues.push_back("Primatives/Geometry/Sphere:Sphere");
            g_app->ReconstructDynamicMenus();

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

    void TK_Loop()
    {
      Timing* timer = &Main::GetInstance()->m_timing;

      while (g_running)
      {
        SDL_Event sdlEvent;
        while (SDL_PollEvent(&sdlEvent))
        {
          PoolEvent(sdlEvent);
          ProcessEvent(sdlEvent);
        }

        timer->CurrentTime = GetElapsedMilliSeconds();
        if (timer->CurrentTime > timer->LastTime + timer->DeltaTime)
        {
          g_app->Frame(timer->CurrentTime - timer->LastTime);

          // Update Present imgui windows.
          ImGui::UpdatePlatformWindows();
          ImGui::RenderPlatformWindowsDefault();
          SDL_GL_MakeCurrent(g_window, g_context);
          SDL_GL_SwapWindow(g_window);

          ClearPool(); // Clear after consumption.

          timer->FrameCount++;
          timer->TimeAccum += timer->CurrentTime - timer->LastTime;
          if (timer->TimeAccum >= 1000.0f)
          {
            g_app->m_fps      = timer->FrameCount;
            timer->TimeAccum  = 0;
            timer->FrameCount = 0;
          }

          timer->LastTime = timer->CurrentTime;
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

#ifdef TK_DEBUG
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  return ToolKit::Editor::ToolKit_Main(argc, argv);
}
