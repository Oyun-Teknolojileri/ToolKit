#include "stdafx.h"

#include "App.h"
#include <stdio.h>
#include <chrono>
#include "SDL.h"
#include "SDL_ttf.h"
#include "Types.h"
#include "Mod.h"
#include "UI.h"
#include "ImGui/imgui_impl_sdl.h"
#include "DebugNew.h"

#define TK_PROFILE

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
		const int width = 1024;
		const int height = 640;

#ifdef TK_PROFILE
		const uint fps = 5000;
#else
		const uint fps = 60;
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
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
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
						glClearColor(0.2f, 0.2f, 0.2f, 1.0);

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

			SDL_DestroyWindow(g_window);
			TTF_Quit();
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
			}

			if (e.type == SDL_DROPFILE)
			{
				UI::ImportData.fullPath = e.drop.file;
				UI::ImportData.showImportWindow = true;
			}

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
				default:
					break;
				}
			}
		}

		int ToolKit_Main(int argc, char* argv[])
		{
			_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

			Init();

			// Continue with editor.
			float lastTime = GetMilliSeconds();
			float currentTime;
			float deltaTime = 1000.0f / fps;
			int frameCount = 0;
			float timeAccum = 0.0f;

			while (g_running)
			{
				SDL_Event sdlEvent;
				while (SDL_PollEvent(&sdlEvent))
				{
					ProcessEvent(sdlEvent);
				}

				currentTime = GetMilliSeconds();
				if (currentTime > lastTime + deltaTime)
				{
					// Update & Render
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					g_app->Frame(currentTime - lastTime);
					SDL_GL_SwapWindow(g_window);

					frameCount++;
					timeAccum += currentTime - lastTime;
					if (timeAccum >= 1000.0f)
					{
						g_app->m_fps = frameCount;
						timeAccum = 0;
						frameCount = 0;
					}

					lastTime = currentTime;
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
	return ToolKit::Editor::ToolKit_Main(argc, argv);
}