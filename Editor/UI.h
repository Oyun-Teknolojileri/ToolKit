#pragma once

#include "ImGui/imgui.h"

namespace ToolKit
{
		namespace Editor
		{
			class Window
			{
			public:
				enum class Type
				{
					Viewport,
					Console
				};

			public:
				Window();
				virtual ~Window();
				virtual void Show() = 0;
				virtual Type GetType() = 0;
				void SetVisibility(bool visible);

				// Window queries.
				bool IsActive();
				bool IsOpen();

			protected:
				// Internal window handling.
				void SetActive();
				void HandleStates();

			protected:
				// States.
				bool m_open = true;
				bool m_active = false;
				bool m_mouseHover = false;
			};

			class UI
			{
			public:
				static void ApplyCustomTheme();
				static void ShowUI();
				static void InitDocking();
				static void ShowAppMainMenuBar();
				static void ShowMenuFile();
				static void ShowMenuWindows();
				static void HelpMarker(const char* desc, float* elapsedHoverTime);
				static void InitIcons();
				static void DispatchSignals();

				// Custom widgets.
				static bool ToggleButton(ImTextureID user_texture_id, const ImVec2& size, bool pushState);

			public:
				static bool m_imguiSampleWindow;
				static bool m_windowMenushowMetrics;
				static float m_hoverTimeForHelp;

				// Toolbar Icons.
				static std::shared_ptr<Texture> m_selectIcn;
				static std::shared_ptr<Texture> m_cursorIcn;
				static std::shared_ptr<Texture> m_moveIcn;
				static std::shared_ptr<Texture> m_rotateIcn;
				static std::shared_ptr<Texture> m_scaleIcn;
				static std::shared_ptr<Texture> m_appIcon;
			};
		}
}
