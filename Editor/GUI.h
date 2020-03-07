#pragma once

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl.h"
#include "ImGui/imgui_impl_opengl3.h"

namespace ToolKit
{
		namespace Editor
		{
			class EditorGUI
			{
			public:
				static void ApplyCustomTheme();
				static void PresentGUI();
				static void InitDocking();
				static void ShowAppMainMenuBar();
				static void ShowMenuFile();
				static void ShowMenuWindows();
				static void HelpMarker(const char* desc, float* elapsedHoverTime);
				static void InitIcons();

				// Custom widgets.
				static bool ToggleButton(ImTextureID user_texture_id, const ImVec2& size, bool pushState);

			public:
				static bool m_windowMenushowMetrics;
				static float m_hoverTimeForHelp;

				// Toolbar Icon
				static std::shared_ptr<Texture> m_selectIcn;
				static std::shared_ptr<Texture> m_cursorIcn;
				static std::shared_ptr<Texture> m_moveIcn;
				static std::shared_ptr<Texture> m_rotateIcn;
				static std::shared_ptr<Texture> m_scaleIcn;
				static std::shared_ptr<Texture> m_appIcon;
			};
		}
}
