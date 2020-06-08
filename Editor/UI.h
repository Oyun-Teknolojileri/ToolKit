#pragma once

#include "ImGui/imgui.h"
#include "Types.h"
#include <functional>

namespace ToolKit
{
	class Texture;

		namespace Editor
		{
			class Window
			{
			public:
				enum class Type
				{
					Viewport,
					Console,
					InputPopup,
					Browser
				};

			public:
				Window();
				virtual ~Window();
				virtual void Show() = 0;
				virtual Type GetType() const = 0;
				void SetVisibility(bool visible);

				// Window queries.
				bool IsActive() const;
				bool IsVisible() const;
				bool MouseHovers() const;
				bool CanDispatchEvents() const; // If active & visible & mouse hovers.

			protected:
				// Internal window handling.
				void HandleStates();
				void SetActive();

			protected:
				// States.
				bool m_visible = true;
				bool m_active = false;
				bool m_mouseHover = false;

			public:
				String m_name;
			};

			class StringInputWindow : public Window
			{
			public:
				StringInputWindow();
				virtual void Show() override;
				virtual Type GetType() const override { return Window::Type::InputPopup; }

			public:
				std::function<void(const String& val)> m_taskFn;
				String m_inputVal;
				String m_inputText;
				String m_hint;
			};

			class YesNoWindow : public Window
			{
			public:
				YesNoWindow(const String& name);
				virtual void Show() override;
				virtual Type GetType() const override { return Window::Type::InputPopup; }

			public:
				std::function<void()> m_yesCallback;
				std::function<void()> m_noCallback;
			};

			class UI
			{
			public:
				static void Init();
				static void UnInit();
				static void InitDocking();
				static void InitIcons();
				static void InitTheme();
				static void ShowUI();
				static void ShowAppMainMenuBar();
				static void ShowMenuFile();
				static void ShowMenuWindows();
				static void ShowImportWindow();
				static void ShowSearchForFilesWindow();
				static void HelpMarker(const char* desc, float* elapsedHoverTime);
				static void DispatchSignals(Window* wnd);
				static void ShowNewSceneWindow();

				// Custom widgets.
				static bool ToggleButton(ImTextureID user_texture_id, const ImVec2& size, bool pushState);

			public:
				static bool m_showNewSceneWindow;
				static bool m_imguiSampleWindow;
				static bool m_windowMenushowMetrics;
				static float m_hoverTimeForHelp;
				static StringInputWindow m_strInputWindow;
				static std::vector<Window*> m_windows;

				static struct Import
				{
					bool showImportWindow = false;
					bool overwrite = false;
					String fullPath;
					String subDir;
					float scale = 1.0f;
				} ImportData;

				static struct SearchFile
				{
					bool showSearchFileWindow = false;
					StringArray missingFiles;
					StringArray searchPaths;
				} SearchFileData;

				// Toolbar Icons.
				static TexturePtr m_selectIcn;
				static TexturePtr m_cursorIcn;
				static TexturePtr m_moveIcn;
				static TexturePtr m_rotateIcn;
				static TexturePtr m_scaleIcn;
				static TexturePtr m_appIcon;
				static TexturePtr m_snapIcon;
				static TexturePtr m_audioIcon;
				static TexturePtr m_cameraIcon;
				static TexturePtr m_clipIcon;
				static TexturePtr m_fileIcon;
				static TexturePtr m_folderIcon;
				static TexturePtr m_imageIcon;
				static TexturePtr m_lightIcon;
				static TexturePtr m_materialIcon;
				static TexturePtr m_meshIcon;
				static TexturePtr m_armatureIcon;
				static TexturePtr m_codeIcon;
				static TexturePtr m_boneIcon;
			};
		}
}
