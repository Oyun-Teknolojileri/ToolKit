#pragma once

#include "ToolKit.h"
#include "GUI.h"

namespace ToolKit
{
	class Camera;
	class RenderTarget;

	namespace Editor
	{
		class OverlayNav;

		enum class ViewportMode
		{
			Idle,
			FpsNavigation
		};

		class Viewport
		{
		public:
			Viewport(float width, float height);
			~Viewport();
			void Update(uint deltaTime);
			void ShowViewport();
			bool IsActive();
			bool IsOpen();

		private:
			void UpdateFpsNavigation(uint deltaTime);
			void OnResize(float width, float height);
			void SetActive();

		public:
			std::string m_name;

			float m_width = 640.0f;
			float m_height = 480.0f;
			Camera* m_camera = nullptr;
			RenderTarget* m_viewportImage = nullptr;
			glm::ivec2 m_lastWndPos;
			static OverlayNav* m_overlayNav;
			
		private:
			static uint m_nextId;

			// States
			bool m_open = true;
			bool m_active = false;
			bool m_relMouseModBegin = true;
			glm::ivec2 m_mousePosBegin;
		};

	}
}
