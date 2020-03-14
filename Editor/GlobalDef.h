#pragma once

#include "SDL.h"
#include "App.h"
#include "Types.h"

namespace ToolKit
{
	namespace Editor
	{

		// Handles.
		extern SDL_Window* g_window;
		extern SDL_GLContext g_context;
		extern Editor::App* g_app;
		extern bool g_running;

		// Colors and materials.
		const std::string g_GRID_MATERIAL_NAME("TK_EDITOR_GRID");
		const glm::vec3 g_gridAxisBlue = glm::vec3(0.196f, 0.541f, 0.905f);
		const glm::vec3 g_gridAxisRed = glm::vec3(0.89f, 0.239f, 0.341f);
		const glm::vec4 g_selectBoxWindowColor = glm::vec4(0.4f, 0.4f, 0.4f, 0.4f);
		const glm::vec4 g_selectBoxBorderColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		// Utility functions.		
		inline float MilisecToSec(float ms)
		{
			return ms / 1000.0f;
		}

		#define GLM2IMVEC(v) *reinterpret_cast<const ImVec2*>(&v)
		#define GLM4IMVEC(v) *reinterpret_cast<const ImVec4*>(&v)
	}
}