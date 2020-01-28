#pragma once

#include "ToolKit.h"
#include "GUI.h"

namespace ToolKit
{
	class Camera;
	class RenderTarget;

	namespace Editor
	{

		class Viewport
		{
		public:
			Viewport(float width, float height);
			~Viewport();
			void Update(uint deltaTime);
			void ShowViewport();

		private:
			void UpdateFpsNavigation(uint deltaTime);
			void OnResize(float width, float height);

		public:
			std::string m_name;

			float m_width = 640.0f;
			float m_height = 480.0f;
			Camera* m_camera = nullptr;
			RenderTarget* m_viewportImage = nullptr;

			// States
			bool m_open = true;
			bool m_active = false;

		private:
			static uint m_nextId;
		};

	}
}
