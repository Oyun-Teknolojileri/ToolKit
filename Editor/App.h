#pragma once

#include "ToolKit.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl.h"
#include "ImGui/imgui_impl_opengl3.h"

extern SDL_Window* g_window;

namespace Editor
{
	using namespace ToolKit;
	using namespace glm;
	using namespace std;

	class App
	{
	public:
		App(int windowWidth, int windowHeight)
			: m_windowWidth(windowWidth), m_windowHeight(windowHeight)
		{
			Main::GetInstance()->Init();
		}

		~App()
		{
			Main::GetInstance()->Uninit();
		}

		void Init()
		{
		}

		void Frame(int deltaTime)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame(g_window);
			ImGui::NewFrame();
			ImGui::ShowDemoWindow();
			ImGui::Render();
		}

	private:
		int m_windowWidth;
		int m_windowHeight;
	};
}
