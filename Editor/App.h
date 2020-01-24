#pragma once

#include "ToolKit.h"
#include "Primative.h"
#include "Renderer.h""
#include "Directional.h"

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
			m_cube.m_mesh->Init();
			m_cam.Translate(vec3(0.0f, 0.0f, 2.0f));
		}

		void Frame(int deltaTime)
		{
			m_renderer.Render(&m_cube, &m_cam);

			//ImGui_ImplOpenGL3_NewFrame();
			//ImGui_ImplSDL2_NewFrame(g_window);
			//ImGui::NewFrame();
			//ImGui::ShowDemoWindow();
			//ImGui::Render();
		}

	private:
		int m_windowWidth;
		int m_windowHeight;

		Cube m_cube;
		Renderer m_renderer;
		Camera m_cam;
	};
}
