#pragma once

#include "ToolKit.h"
#include "Primative.h"
#include "Renderer.h"
#include "Directional.h"

#include "GUI.h"

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

			EditorGUI::PresentGUI();
		}

		void OnResize(int width, int height)
		{
			m_windowWidth = width;
			m_windowHeight = height;
			m_cam.SetLens(glm::radians(90.0f), (float)m_windowWidth, (float)m_windowHeight, 0.01f, 1000.0f);
		}

	private:
		int m_windowWidth;
		int m_windowHeight;

		Cube m_cube;
		Renderer m_renderer;
		Camera m_cam;
	};
}
