#include "stdafx.h"
#include "ToolKit.h"
#include "DllMain.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include <chrono>

// Setup.
MainParams g_args;

// Global handels.
SDL_Window* g_window = nullptr;
SDL_GLContext g_context = nullptr;

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

		int visFlag = g_args.hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN;

		g_window = SDL_CreateWindow(g_args.programName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, g_args.windowWidth, g_args.windowHeight, SDL_WINDOW_OPENGL | visFlag);
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

				ToolKit::Main::GetInstance()->Init();

				// Set defaults
				SDL_GL_SetSwapInterval(1);
				glEnable(GL_CULL_FACE);
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_MULTISAMPLE);
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
	ToolKit::Main::GetInstance()->Uninit();

	SDL_DestroyWindow(g_window);
	TTF_Quit();
	SDL_Quit();
}

void ScreenShot(int x, int y, int w, int h, std::string filename)
{
	unsigned char* pixels = new unsigned char[w * h * 4]; // 4 bytes for RGBA
	glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(pixels, w, h, 8 * 4, w * 4, 0, 0, 0, 0);
	SDL_SaveBMP(surf, filename.c_str());

	SDL_FreeSurface(surf);
	delete[] pixels;
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

extern "C"
{
	ABT_EXPORT DllErrorCodes AbtMain(MainParams args)
	{
		g_args = args;
		Init();

		unsigned int lastTime = GetMilliSeconds();
		unsigned int currentTime;
		unsigned int deltaTime = 1000 / g_args.fps;

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
				int deltaTime = currentTime - lastTime;
				// g_args.FrameCallback(deltaTime);
				SDL_GL_SwapWindow(g_window);

				lastTime = currentTime;
			}
			else
			{
				SDL_Delay(10);
			}
		}

		Exit();

		return DllErrorCodes::Succeeded;
	}
}