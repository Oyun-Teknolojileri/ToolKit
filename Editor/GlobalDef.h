#pragma once

#include "SDL.h"
#include "App.h"
#include "Types.h"

namespace ToolKit
{
	namespace Editor
	{

		extern SDL_Window* g_window;
		extern SDL_GLContext g_context;
		extern Editor::App* g_app;
		extern bool g_running;

		inline float MilisecToSec(uint ms)
		{
			return ms / 1000.0f;
		}

	}
}