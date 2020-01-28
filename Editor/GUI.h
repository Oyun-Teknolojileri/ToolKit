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
				static void ShowSimpleWindow();
				static void InitDocking();
				static void ShowAppMainMenuBar();
				static void ShowMenuFile();
				static void ShowMenuWindows();
			};

		}
}
