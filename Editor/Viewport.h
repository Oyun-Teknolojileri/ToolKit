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
			virtual Type GetType() const override;
			void Update(float deltaTime);

			// Window queries.
			bool IsViewportQueriable();

			// Utility Functions.
			Ray RayFromMousePosition();
			Ray RayFromScreenSpacePoint(const Vec2& pnt);
			Vec3 GetLastMousePosWorldSpace();
			Vec2 GetLastMousePosViewportSpace();
			Vec2 GetLastMousePosScreenSpace();
			Vec3 TransformViewportToWorldSpace(const Vec2& pnt);
			Vec2 TransformScreenToViewportSpace(const Vec2& pnt);

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
			float m_width = 640.0f;
			float m_height = 480.0f;
			Vec2 m_wndPos;
			Vec2 m_wndContentAreaSize;
			bool m_orthographic = false;

			static class OverlayMods* m_overlayMods;
			static class OverlayViewportOptions* m_overlayOptions;

			// UI Draw commands.
			std::vector<std::function<void(ImDrawList*)>> m_drawCommands;
			
		private:
			static uint m_nextId;

			// States.
			bool m_mouseOverContentArea = false;
			bool m_relMouseModBegin = true;

			glm::ivec2 m_mousePosBegin;
			glm::ivec2 m_lastMousePosRelContentArea;
		};

	}
}
