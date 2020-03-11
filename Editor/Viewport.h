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

		class DrawUI
		{
		public:
			enum class DrawCode
			{
				FilledRectangle
			};

		public:
			virtual void Draw() = 0;

		public:
			DrawCode m_code;
		};

		class DrawUIFilledRectangle : public DrawUI
		{
		public:
			DrawUIFilledRectangle(glm::vec2 min, glm::vec2 max, glm::vec4 col, float rounding)
			{
				m_code = DrawCode::FilledRectangle;
				m_min = min;
				m_max = max;
				m_col = col;
				m_rounding = rounding;
			}
			
			virtual void Draw()
			{
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				ImVec4 colf = ImVec4(m_col.r, m_col.g, m_col.b, m_col.a);
				const ImU32 col = ImColor(colf);
				drawList->AddRectFilled(ImVec2(m_min.x, m_min.y), ImVec2(m_max.x, m_max.y), col, m_rounding);
			}

		public:
			glm::vec2 m_min;
			glm::vec2 m_max;
			glm::vec4 m_col;
			float m_rounding;
		};

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
			Camera* m_camera = nullptr;
			RenderTarget* m_viewportImage = nullptr;

			// Window properties.
			std::string m_name;
			float m_width = 640.0f;
			float m_height = 480.0f;
			glm::ivec2 m_wndPos;
			glm::vec2 m_wndContentAreaSize;

			static OverlayNav* m_overlayNav;

			// UI Draw Queue
			std::vector<DrawUI*> m_drawQueue;
			
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
