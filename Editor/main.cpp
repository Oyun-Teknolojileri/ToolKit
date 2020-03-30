#include "stdafx.h"

#include "App.h"
#include <stdio.h>
#include <chrono>
#include "SDL.h"
#include "SDL_ttf.h"
#include "UI.h"
#include "Types.h"
#include "Mod.h"
#include "DebugNew.h"

// Global handles.
namespace ToolKit
{
	namespace Editor
	{
		bool g_running = true;
		SDL_Window* g_window = nullptr;
		SDL_GLContext g_context = nullptr;
		App* g_app;
	}
}

// Setup.
const char* appName = "ToolKit";
const int width = 1024;
const int height = 640;
const ToolKit::uint fps = 60;

void Init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
	{
		ToolKit::Editor::g_running = false;
	}
	else
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

		ToolKit::Editor::g_window = SDL_CreateWindow(appName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
		if (ToolKit::Editor::g_window == nullptr)
		{
			ToolKit::Editor::g_running = false;
		}
		else
		{
			ToolKit::Editor::g_context = SDL_GL_CreateContext(ToolKit::Editor::g_window);
			if (ToolKit::Editor::g_context == nullptr)
			{
				ToolKit::Editor::g_running = false;
			}
			else
			{
				// Init glew
				glewExperimental = true;
				GLenum err = glewInit();
				if (GLEW_OK != err)
				{
					ToolKit::Editor::g_running = false;
					return;
				}

				if (TTF_Init() == -1)
					return;

				ToolKit::Main::GetInstance()->Init();

				// Set defaults
				SDL_GL_SetSwapInterval(1);
				glClearColor(0.2f, 0.2f, 0.2f, 1.0);
				
				glEnable(GL_CULL_FACE);
				glEnable(GL_DEPTH_TEST);

				// Init app
				ToolKit::Editor::g_app = new ToolKit::Editor::App(width, height);
				ToolKit::Editor::g_app->Init();
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
	ToolKit::Editor::g_running = false;
	SafeDel(ToolKit::Editor::g_app);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyWindow(ToolKit::Editor::g_window);
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
			ToolKit::Editor::g_app->OnResize(e.window.data1, e.window.data2);
		}
	}

	if (e.type == SDL_QUIT)
	{
		ToolKit::Editor::g_running = false;
	}

	if (e.type == SDL_KEYDOWN)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			ToolKit::Editor::g_running = false;
			break;
		default:
			break;
		}
	}
}

int main(int argc, char* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Init();

	// Init imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	ToolKit::Editor::UI::ApplyCustomTheme();

	ImGui_ImplSDL2_InitForOpenGL(ToolKit::Editor::g_window, ToolKit::Editor::g_context);
	ImGui_ImplOpenGL3_Init("#version 300 es");

	// Continue with editor.
	float lastTime = GetMilliSeconds();
	float currentTime;
	float deltaTime = 1000.0f / fps;

	while (ToolKit::Editor::g_running)
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
			ToolKit::Editor::g_app->Frame(currentTime - lastTime);
			SDL_GL_SwapWindow(ToolKit::Editor::g_window);

			lastTime = currentTime;
		}
		else
		{
			SDL_Delay(10);
		}
	}

	Exit();

	return 0;
}
