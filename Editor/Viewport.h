#pragma once

#include "ToolKit.h"
#include "UI.h"
#include "MathUtil.h"
#include <functional>

namespace ToolKit
{
	class Camera;
	class RenderTarget;

	namespace Editor
	{
		class Viewport : public Window
		{
		public:
			Viewport(float width, float height);
			virtual ~Viewport();
			virtual void Show() override;
			virtual Type GetType() override;
			void Update(float deltaTime);

			// Window queries.
			bool IsViewportQueriable();

			// Utility Functions.
			Ray RayFromMousePosition();
			glm::vec3 GetLastMousePosWorldSpace();
			glm::vec2 GetLastMousePosViewportSpace();
			glm::vec2 GetLastMousePosScreenSpace();
			glm::vec3 TransformViewportToWorldSpace(const glm::vec2& pnt);
			glm::vec2 TransformScreenToViewportSpace(const glm::vec2& pnt);

		private:
			// Mods.
			void FpsNavigationMode(float deltaTime);

			// Internal window handling.
			void OnResize(float width, float height);

		public:
			// ToolKit bindings.
			class Camera* m_camera = nullptr;
			class RenderTarget* m_viewportImage = nullptr;

			// Window properties.
			std::string m_name;
			float m_width = 640.0f;
			float m_height = 480.0f;
			glm::vec2 m_wndPos;
			glm::vec2 m_wndContentAreaSize;

			static class OverlayNav* m_overlayNav;

			// UI Draw commands.
			std::vector<std::function<void(ImDrawList*)>> m_drawCommands;
			
		private:
			static uint m_nextId;

			// States.
			bool m_mouseHover = false;
			bool m_mouseOverContentArea = false;
			bool m_open = true;
			bool m_active = false;
			bool m_relMouseModBegin = true;

			glm::ivec2 m_mousePosBegin;
			glm::ivec2 m_lastMousePosRelContentArea;
		};

	}
}
