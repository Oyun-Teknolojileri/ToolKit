#include "App.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include <chrono>
#include "DebugNew.h"

// Setup.
const char* appName = "Editor";
const int width = 640;
const int height = 480;
const unsigned int fps = 60;

// Global handles.
SDL_Window* g_window = nullptr;
SDL_GLContext g_context = nullptr;
Editor::App* g_app;
bool g_running = true;

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

		g_window = SDL_CreateWindow(appName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
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

				if (TTF_Init() == -1)
					return;

				// Set defaults
				SDL_GL_SetSwapInterval(1);
				glEnable(GL_CULL_FACE);
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_MULTISAMPLE);

				// Init app
				g_app = new Editor::App(width, height);
				g_app->Init();
			}
		}
	}
}

unsigned long GetMilliSeconds()
{
	using namespace std::chrono;

	static high_resolution_clock::time_point t1 = high_resolution_clock::now();
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

	return (unsigned long)(time_span.count() * 1000.0);
}

void Exit()
{
	g_running = false;
	SafeDel(g_app);

	SDL_DestroyWindow(g_window);
	TTF_Quit();
	SDL_Quit();
}

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
}

int main(int argc, char* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Init();

	unsigned int lastTime = GetMilliSeconds();
	unsigned int currentTime;
	unsigned int deltaTime = 1000 / fps;

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
