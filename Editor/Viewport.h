#pragma once

#include "ToolKit.h"
#include "GUI.h"
#include "MathUtil.h"
#include <functional>

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
			void Update(float deltaTime);

			// Window queries.
			void ShowViewport();
			bool IsActive();
			bool IsOpen();
			bool IsViewportQueriable();

			// Utility Functions.
			Ray RayFromMousePosition();
			glm::vec3 GetLastMousePosInWorldSpace();
			glm::vec2 GetLastMousePosWindowSpace();

		private:
			// Modes.
			void FpsNavigationMode(float deltaTime);

			// Internal window handlings.
			void OnResize(float width, float height);
			void SetActive();

		public:
			// ToolKit bindings.
			class Camera* m_camera = nullptr;
			class RenderTarget* m_viewportImage = nullptr;

			// Window properties.
			std::string m_name;
			float m_width = 640.0f;
			float m_height = 480.0f;
			glm::ivec2 m_wndPos;
			glm::vec2 m_wndContentAreaSize;

			static class OverlayNav* m_overlayNav;

			// UI Draw commands.
			std::vector<std::function<void(ImDrawList*)>> m_drawCommands;
			
		private:
			static uint m_nextId;

			// States
			bool m_mouseOverContentArea = false;
			bool m_open = true;
			bool m_active = false;
			bool m_relMouseModBegin = true;

			glm::ivec2 m_mousePosBegin;
			glm::ivec2 m_lastMousePosRelContentArea;
		};

	}
}
