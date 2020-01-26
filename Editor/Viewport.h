#pragma once

#include "ToolKit.h"
#include "GUI.h"

namespace ToolKit
{
	class Camera;
	class RenderTarget;
}

namespace Editor
{

	using namespace ToolKit;

	class Viewport
	{
	public:
		Viewport(float width, float height);
		~Viewport();

		void OnResize(float width, float height);
		void ShowViewport();

	public:
		std::string m_name;

		float m_width = 640.0f;
		float m_height = 480.0f;
		Camera* m_camera = nullptr;
		RenderTarget* m_viewportImage = nullptr;
		bool m_open = true;

	private:
		static uint m_nextId;
	};

}