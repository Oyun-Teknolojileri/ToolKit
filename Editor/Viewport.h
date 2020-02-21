#pragma once

#include "ToolKit.h"
#include "GUI.h"
#include "MathUtil.h"

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

			// Window queries.
			void ShowViewport();
			bool IsActive();
			bool IsOpen();
			bool IsViewportQueriable();

			// Utility Functions.
			Ray RayFromMousePosition();

		private:
			// Modes.
			void PickObjectMode();
			void FpsNavigationMode(uint deltaTime);

			// Internal window handlings.
			void OnResize(float width, float height);
			void SetActive();

		public:
			// ToolKit bindings.
			Camera* m_camera = nullptr;
			RenderTarget* m_viewportImage = nullptr;

			// Window properties.
			std::string m_name;
			float m_width = 640.0f;
			float m_height = 480.0f;
			glm::ivec2 m_wndPos;
			glm::vec2 m_wndContentAreaSize;

			static OverlayNav* m_overlayNav;
			
		private:
			static uint m_nextId;

			// States
			bool m_mouseOverContentArea = false;
			bool m_open = true;
			bool m_active = false;
			bool m_relMouseModBegin = true;
			bool m_pickingDebug = false;

			glm::ivec2 m_mousePosBegin;
			glm::ivec2 m_lastMousePosRelContentArea;
		};

	}
}
