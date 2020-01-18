#pragma once

#include "ToolKit.h"

namespace $safeprojectname$
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
		}

	private:
		int m_windowWidth;
		int m_windowHeight;
	};
}
