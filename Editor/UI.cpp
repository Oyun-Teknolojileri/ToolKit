#include "stdafx.h"

#include "UI.h"

#include "ImGui/imgui_impl_sdl.h"
#include "ImGui/imgui_impl_opengl3.h"
#include "ImGui/imgui_stdlib.h"

#include "App.h"
#include "Viewport.h"
#include "SDL.h"
#include "GlobalDef.h"
#include "Mod.h"
#include "ConsoleWindow.h"
#include "FolderWindow.h"
#include "OverlayUI.h"
#include "DebugNew.h"

namespace ToolKit
{
	namespace Editor
	{

		bool UI::m_windowMenushowMetrics = false;
		bool UI::m_imguiSampleWindow = false;
		bool UI::m_showNewSceneWindow = false;
		float UI::m_hoverTimeForHelp = 1.0f;
		UI::Import UI::ImportData;
		UI::SearchFile UI::SearchFileData;
		StringInputWindow UI::m_strInputWindow;
		std::vector<Window*> UI::m_volatileWindows;

		// Icons
		TexturePtr UI::m_selectIcn;
		TexturePtr UI::m_cursorIcn;
		TexturePtr UI::m_moveIcn;
		TexturePtr UI::m_rotateIcn;
		TexturePtr UI::m_scaleIcn;
		TexturePtr UI::m_appIcon;
		TexturePtr UI::m_snapIcon;
		TexturePtr UI::m_audioIcon;
		TexturePtr UI::m_cameraIcon;
		TexturePtr UI::m_clipIcon;
		TexturePtr UI::m_fileIcon;
		TexturePtr UI::m_folderIcon;
		TexturePtr UI::m_imageIcon;
		TexturePtr UI::m_lightIcon;
		TexturePtr UI::m_materialIcon;
		TexturePtr UI::m_meshIcon;
		TexturePtr UI::m_armatureIcon;
		TexturePtr UI::m_codeIcon;
		TexturePtr UI::m_boneIcon;
		TexturePtr UI::m_worldIcon;
		TexturePtr UI::m_axisIcon;

		void UI::Init()
		{
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
			io.ConfigWindowsMoveFromTitleBarOnly = true;

			ImGui_ImplSDL2_InitForOpenGL(g_window, g_context);
			ImGui_ImplOpenGL3_Init("#version 300 es");

			InitIcons();
			InitTheme();
		}

		void UI::UnInit()
		{
			for (size_t i = 0; i < m_volatileWindows.size(); i++)
			{
				SafeDel(m_volatileWindows[i]);
			}
			assert(m_volatileWindows.size() < 10 && "Overflowing danger.");
			m_volatileWindows.clear();

			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplSDL2_Shutdown();
			ImGui::DestroyContext();
		}

		void UI::InitDocking()
		{
			static bool opt_fullscreen_persistant = true;
			bool opt_fullscreen = opt_fullscreen_persistant;
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			if (opt_fullscreen)
			{
				ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(viewport->Pos);
				ImGui::SetNextWindowSize(viewport->Size);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			}

			if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("DockSpace Demo", nullptr, window_flags);
			ImGui::PopStyleVar();

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);

			// DockSpace
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}

			ImGui::End();
		}

		void UI::InitIcons()
		{
			m_selectIcn = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/select.png"));
			m_selectIcn->Init();
			m_cursorIcn = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/cursor.png"));
			m_cursorIcn->Init();
			m_moveIcn = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/move.png"));
			m_moveIcn->Init();
			m_rotateIcn = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/rotate.png"));
			m_rotateIcn->Init();
			m_scaleIcn = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/scale.png"));
			m_scaleIcn->Init();
			m_snapIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/snap.png"));
			m_snapIcon->Init();
			m_audioIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/audio.png"));
			m_audioIcon->Init();
			m_cameraIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/camera.png"));
			m_cameraIcon->Init();
			m_clipIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/clip.png"));
			m_clipIcon->Init();
			m_fileIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/file.png"));
			m_fileIcon->Init();
			m_folderIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/folder.png"));
			m_folderIcon->Init();
			m_imageIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/image.png"));
			m_imageIcon->Init();
			m_lightIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/light.png"));
			m_lightIcon->Init();
			m_materialIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/material.png"));
			m_materialIcon->Init();
			m_meshIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/mesh.png"));
			m_meshIcon->Init();
			m_armatureIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/armature.png"));
			m_armatureIcon->Init();
			m_codeIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/code.png"));
			m_codeIcon->Init();
			m_boneIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/bone.png"));
			m_boneIcon->Init();
			m_worldIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/world.png"));
			m_worldIcon->Init();
			m_axisIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/axis.png"));
			m_axisIcon->Init();

			// Set application Icon.
			m_appIcon = Main::GetInstance()->m_textureMan.Create(TexturePath("Icons/app.png"));
			m_appIcon->Init(false);
			SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(m_appIcon->m_image, m_appIcon->m_width, m_appIcon->m_height, 8, m_appIcon->m_width * 4, SDL_PIXELFORMAT_ABGR8888);
			SDL_SetWindowIcon(g_window, surface);
			SDL_FreeSurface(surface);
		}

		void UI::InitTheme()
		{
			ImGuiStyle* style = &ImGui::GetStyle();
			style->WindowRounding = 5.3f;
			style->GrabRounding = style->FrameRounding = 2.3f;
			style->ScrollbarRounding = 5.0f;
			style->FrameBorderSize = 1.0f;
			style->ItemSpacing.y = 6.5f;

			style->Colors[ImGuiCol_Text] = { 0.73333335f, 0.73333335f, 0.73333335f, 1.00f };
			style->Colors[ImGuiCol_TextDisabled] = { 0.34509805f, 0.34509805f, 0.34509805f, 1.00f };
			style->Colors[ImGuiCol_WindowBg] = { 0.23529413f, 0.24705884f, 0.25490198f, 0.94f };
			style->Colors[ImGuiCol_ChildBg] = { 0.23529413f, 0.24705884f, 0.25490198f, 0.00f };
			style->Colors[ImGuiCol_PopupBg] = { 0.23529413f, 0.24705884f, 0.25490198f, 0.94f };
			style->Colors[ImGuiCol_Border] = { 0.33333334f, 0.33333334f, 0.33333334f, 0.50f };
			style->Colors[ImGuiCol_BorderShadow] = { 0.15686275f, 0.15686275f, 0.15686275f, 0.00f };
			style->Colors[ImGuiCol_FrameBg] = { 0.16862746f, 0.16862746f, 0.16862746f, 0.54f };
			style->Colors[ImGuiCol_FrameBgHovered] = { 0.453125f, 0.67578125f, 0.99609375f, 0.67f };
			style->Colors[ImGuiCol_FrameBgActive] = { 0.47058827f, 0.47058827f, 0.47058827f, 0.67f };
			style->Colors[ImGuiCol_TitleBg] = { 0.04f, 0.04f, 0.04f, 1.00f };
			style->Colors[ImGuiCol_TitleBgCollapsed] = { 0.16f, 0.29f, 0.48f, 1.00f };
			style->Colors[ImGuiCol_TitleBgActive] = { 0.00f, 0.00f, 0.00f, 0.51f };
			style->Colors[ImGuiCol_MenuBarBg] = { 0.27058825f, 0.28627452f, 0.2901961f, 0.80f };
			style->Colors[ImGuiCol_ScrollbarBg] = { 0.27058825f, 0.28627452f, 0.2901961f, 0.60f };
			style->Colors[ImGuiCol_ScrollbarGrab] = { 0.21960786f, 0.30980393f, 0.41960788f, 0.51f };
			style->Colors[ImGuiCol_ScrollbarGrabHovered] = { 0.21960786f, 0.30980393f, 0.41960788f, 1.00f };
			style->Colors[ImGuiCol_ScrollbarGrabActive] = { 0.13725491f, 0.19215688f, 0.2627451f, 0.91f };
			// style->Colors[ImGuiCol_ComboBg]               = {0.1f, 0.1f, 0.1f, 0.99f};
			style->Colors[ImGuiCol_CheckMark] = { 0.90f, 0.90f, 0.90f, 0.83f };
			style->Colors[ImGuiCol_SliderGrab] = { 0.70f, 0.70f, 0.70f, 0.62f };
			style->Colors[ImGuiCol_SliderGrabActive] = { 0.30f, 0.30f, 0.30f, 0.84f };
			style->Colors[ImGuiCol_Button] = { 0.33333334f, 0.3529412f, 0.36078432f, 0.49f };
			style->Colors[ImGuiCol_ButtonHovered] = { 0.21960786f, 0.30980393f, 0.41960788f, 1.00f };
			style->Colors[ImGuiCol_ButtonActive] = { 0.13725491f, 0.19215688f, 0.2627451f, 1.00f };
			style->Colors[ImGuiCol_Header] = { 0.33333334f, 0.3529412f, 0.36078432f, 0.53f };
			style->Colors[ImGuiCol_HeaderHovered] = { 0.453125f, 0.67578125f, 0.99609375f, 0.67f };
			style->Colors[ImGuiCol_HeaderActive] = { 0.47058827f, 0.47058827f, 0.47058827f, 0.67f };
			style->Colors[ImGuiCol_Separator] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
			style->Colors[ImGuiCol_SeparatorHovered] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
			style->Colors[ImGuiCol_SeparatorActive] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
			style->Colors[ImGuiCol_ResizeGrip] = { 1.00f, 1.00f, 1.00f, 0.85f };
			style->Colors[ImGuiCol_ResizeGripHovered] = { 1.00f, 1.00f, 1.00f, 0.60f };
			style->Colors[ImGuiCol_ResizeGripActive] = { 1.00f, 1.00f, 1.00f, 0.90f };
			style->Colors[ImGuiCol_PlotLines] = { 0.61f, 0.61f, 0.61f, 1.00f };
			style->Colors[ImGuiCol_PlotLinesHovered] = { 1.00f, 0.43f, 0.35f, 1.00f };
			style->Colors[ImGuiCol_PlotHistogram] = { 0.90f, 0.70f, 0.00f, 1.00f };
			style->Colors[ImGuiCol_PlotHistogramHovered] = { 1.00f, 0.60f, 0.00f, 1.00f };
			style->Colors[ImGuiCol_TextSelectedBg] = { 0.18431373f, 0.39607847f, 0.79215693f, 0.90f };
		}

		void UI::ShowUI()
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame(g_window);
			ImGui::NewFrame();

			InitDocking();
			ShowAppMainMenuBar();

			for (Window* wnd : g_app->m_windows)
			{
				if (wnd->IsVisible())
				{
					wnd->Show();
				}
			}

			if (m_imguiSampleWindow)
			{
				ImGui::ShowDemoWindow(&m_imguiSampleWindow);
			}

			if (m_windowMenushowMetrics)
			{
				ImGui::ShowMetricsWindow(&m_windowMenushowMetrics);
			}

			ShowImportWindow();
			ShowSearchForFilesWindow();
			ShowNewSceneWindow();

			m_strInputWindow.Show();

			// Show & Destroy if not visible.
			for (int i = (int)m_volatileWindows.size() - 1; i > -1; i--)
			{
				Window* wnd = m_volatileWindows[i];
				if (wnd->IsVisible())
				{
					wnd->Show();
				}
				else
				{
					SafeDel(wnd);
					m_volatileWindows.erase(m_volatileWindows.begin() + i);
				}
			}

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			ImGui::EndFrame();

			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			SDL_GL_MakeCurrent(g_window, g_context);
		}

		void UI::ShowAppMainMenuBar()
		{
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					ShowMenuFile();
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Windows"))
				{
					ShowMenuWindows();
					ImGui::EndMenu();
				}

				ImGui::EndMainMenuBar();
			}
		}

		void UI::ShowMenuFile()
		{
			if (ImGui::MenuItem("NewScene"))
			{
				m_strInputWindow.m_name = "NewScene##NwScn1";
				m_strInputWindow.m_inputText = "Name";
				m_strInputWindow.m_hint = "Scene name";
				m_strInputWindow.SetVisibility(true);
				m_strInputWindow.m_taskFn = [](const String& val)
				{
					g_app->OnNewScene(val);
				};
			}

			if (ImGui::MenuItem("Save"))
			{
				XmlDocument doc;
				g_app->m_scene.Serialize(&doc, nullptr);
			}

			if (ImGui::MenuItem("SaveAs"))
			{
				m_strInputWindow.m_name = "SaveScene##SvScn1";
				m_strInputWindow.m_inputText = "Name";
				m_strInputWindow.m_hint = "Scene name";
				m_strInputWindow.SetVisibility(true);
				m_strInputWindow.m_taskFn = [](const String& val)
				{
					XmlDocument doc;
					g_app->m_scene.m_name = val;
					g_app->m_scene.Serialize(&doc, nullptr);
				};
			}

			if (ImGui::MenuItem("Quit", "Alt+F4"))
			{
				g_app->OnQuit();
			}
		}

		void UI::ShowMenuWindows()
		{
			if (ImGui::BeginMenu("Viewport"))
			{
				for (int i = (int)g_app->m_windows.size() - 1; i >= 0; i--)
				{
					Window* wnd = g_app->m_windows[i];
					if (wnd->GetType() != Window::Type::Viewport)
					{
						continue;
					}

					Viewport* vp = static_cast<Viewport*> (wnd);
					if (ImGui::MenuItem(vp->m_name.c_str(), nullptr, false, !vp->IsVisible()))
					{
						vp->SetVisibility(true);
					}

					if (vp->IsVisible())
					{
						ImGui::SameLine();
						if (ImGui::Button("x"))
						{
							g_app->m_windows.erase(g_app->m_windows.begin() + i);
							SafeDel(vp);
							continue;
						}
					}
				}

				if (ImGui::MenuItem("Add Viewport", "Alt+V"))
				{
					Viewport* vp = new Viewport(640, 480);
					g_app->m_windows.push_back(vp);
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Console Window", "Alt+C", nullptr, !g_app->GetConsole()->IsVisible()))
			{
				g_app->GetConsole()->SetVisibility(true);
			}

			if (ImGui::MenuItem("Resource Window", "Alt+B", nullptr, !g_app->GetAssetBrowser()->IsVisible()))
			{
				g_app->GetAssetBrowser()->SetVisibility(true);
			}

			ImGui::Separator();

			if (!m_windowMenushowMetrics)
			{
				if (ImGui::MenuItem("Show Metrics", "Alt+M"))
				{
					m_windowMenushowMetrics = true;
				}
			}

			if (!m_imguiSampleWindow)
			{
				if (ImGui::MenuItem("Imgui Sample", "Alt+S"))
				{
					m_imguiSampleWindow = true;
				}
			}
		}

		void UI::ShowImportWindow()
		{
			if (!ImportData.showImportWindow)
			{
				return;
			}

			if (g_app->m_importSlient)
			{
				for (size_t i = 0; i < ImportData.files.size(); i++)
				{
					g_app->Import(ImportData.files[i], ImportData.subDir, ImportData.overwrite);
				}
				ImportData.files.clear();
				ImportData.showImportWindow = false;
			}

			ImGui::OpenPopup("Import");
			if (ImGui::BeginPopupModal("Import", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Import file: \n\n");
				for (size_t i = 0; i < ImportData.files.size(); i++)
				{
					ImGui::Text(ImportData.files[i].c_str());
				}
				ImGui::Separator();

				StringArray fails;
				if (!ImportData.files.empty())
				{
					for (int i = (int)ImportData.files.size() - 1; i >= 0; i--)
					{
						bool canImp = g_app->CanImport(ImportData.files[i]);
						if (!canImp)
						{
							fails.push_back(ImportData.files[i]);
							ImportData.files.erase(ImportData.files.begin() + i);
						}
					}
				}

				if (!fails.empty())
				{
					ImGui::Text("Fallowing imports failed due to:\nFile format is not supported.\nSuported formats are fbx, glb, obj.");
					for (String& file : fails)
					{
						ImGui::Text(file.c_str());
					}
					ImGui::Separator();
				}

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
				ImGui::Checkbox("Override", &ImportData.overwrite);
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("Scale", &ImportData.scale);
				ImGui::PopItemWidth();
				ImGui::PopStyleVar();

				ImGui::InputTextWithHint("Subdir", "optional", &ImportData.subDir);

				if (ImGui::Button("OK", ImVec2(120, 0))) 
				{ 
					for (size_t i = 0; i < ImportData.files.size(); i++)
					{
						if (g_app->Import(ImportData.files[i], ImportData.subDir, ImportData.overwrite) == -1)
						{
							// Fall back to search.
							ImGui::EndPopup();
							ImportData.showImportWindow = false;
							return;
						}
					}
					ImportData.files.clear();
					ImportData.showImportWindow = false;
					ImGui::CloseCurrentPopup(); 
				}
				ImGui::SetItemDefaultFocus();
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0))) 
				{ 
					ImportData.files.clear();
					ImportData.showImportWindow = false;
					ImGui::CloseCurrentPopup(); 
				}

				ImGui::EndPopup();
			}
		}

		void UI::ShowSearchForFilesWindow()
		{
			if (!SearchFileData.showSearchFileWindow)
			{
				return;
			}

			ImGui::OpenPopup("SearchFile");
			if (ImGui::BeginPopupModal("SearchFile", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				static String buffer;
				float maxSize = 400.0f;
				ImGui::PushItemWidth(maxSize);
				ImGui::InputText("SearchPath", &buffer);
				ImGui::PopItemWidth();
				ImGui::SameLine();
				
				if (ImGui::Button("Add"))
				{
					if (!buffer.empty())
					{
						SearchFileData.searchPaths.push_back(buffer);
					}
				}
				
				ImGui::Separator();
				ImGui::Text("MissingFiles:");
				for (size_t i = 0; i < SearchFileData.missingFiles.size(); i++)
				{
					String* s = &SearchFileData.missingFiles[i];
					ImGui::Text(s->c_str());
				}

				int itemCnt = (int)SearchFileData.searchPaths.size();
				const char** items = new const char*[itemCnt];
				for (int i = 0; i < itemCnt; i++)
				{
					items[i] = SearchFileData.searchPaths[i].c_str();
					float textSize = ImGui::CalcTextSize(items[i]).x;
					maxSize = glm::max(textSize, maxSize);
				}

				ImGui::Separator();
				ImGui::Text("SearchPaths:");
				static int currItem = 0;
				ImGui::PushItemWidth(maxSize * 1.1f);
				ImGui::ListBox("##1", &currItem, items, itemCnt, 4);
				ImGui::PopItemWidth();
				SafeDelArray(items);

				if (ImGui::Button("Remove"))
				{
					if (currItem < itemCnt)
					{
						SearchFileData.searchPaths.erase(SearchFileData.searchPaths.begin() + currItem);
					}
				}

				if (ImGui::Button("Search", ImVec2(120, 0)))
				{
					for (size_t i = 0; i < ImportData.files.size(); i++)
					{
						g_app->Import(ImportData.files[i], ImportData.subDir, ImportData.overwrite);
					}
					ImportData.files.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SetItemDefaultFocus();
				ImGui::SameLine();
				if (ImGui::Button("Abort", ImVec2(120, 0)))
				{
					ImportData.files.clear();
					SearchFileData.showSearchFileWindow = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		void UI::HelpMarker(const char* desc, float* elapsedHoverTime)
		{
			if (ImGui::IsItemHovered())
			{
				*elapsedHoverTime += ImGui::GetIO().DeltaTime;
				if (UI::m_hoverTimeForHelp > * elapsedHoverTime)
				{
					return;
				}

				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(desc);
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
			else
			{
				*elapsedHoverTime = 0.0f;
			}
		}

		void UI::DispatchSignals(Window* wnd)
		{
			if (wnd == nullptr)
			{
				return;
			}

			if (!wnd->CanDispatchEvents())
			{
				return;
			}

			ImGuiIO& io = ImGui::GetIO();
			if (io.MouseClicked[ImGuiMouseButton_Left])
			{
				ModManager::GetInstance()->DispatchSignal(BaseMod::m_leftMouseBtnDownSgnl);
			}

			if (io.MouseReleased[ImGuiMouseButton_Left])
			{
				ModManager::GetInstance()->DispatchSignal(BaseMod::m_leftMouseBtnUpSgnl);
			}

			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				ModManager::GetInstance()->DispatchSignal(BaseMod::m_leftMouseBtnDragSgnl);
			}

			if (io.KeysDown[io.KeyMap[ImGuiKey_Delete]])
			{
				if (io.KeysDownDuration[io.KeyMap[ImGuiKey_Delete]] == 0.0f)
				{
					ModManager::GetInstance()->DispatchSignal(BaseMod::m_delete);
				}
			}

			if (io.KeysDown[SDL_SCANCODE_D] && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				if (io.KeysDownDuration[SDL_SCANCODE_D] == 0.0f)
				{
					ModManager::GetInstance()->DispatchSignal(BaseMod::m_duplicate);
				}
			}

			if (io.KeysDown[SDL_SCANCODE_B] && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				if (io.KeysDownDuration[SDL_SCANCODE_B] == 0.0f)
				{
					ModManager::GetInstance()->SetMod(true, ModId::Select);
				}
			}

			if (io.KeysDown[SDL_SCANCODE_S] && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				if (io.KeysDownDuration[SDL_SCANCODE_S] == 0.0f)
				{
					ModManager::GetInstance()->SetMod(true, ModId::Scale);
				}
			}

			if (io.KeysDown[SDL_SCANCODE_R] && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				if (io.KeysDownDuration[SDL_SCANCODE_R] == 0.0f)
				{
					ModManager::GetInstance()->SetMod(true, ModId::Rotate);
				}
			}

			if (io.KeysDown[SDL_SCANCODE_G] && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				if (io.KeysDownDuration[SDL_SCANCODE_G] == 0.0f)
				{
					ModManager::GetInstance()->SetMod(true, ModId::Move);
				}
			}

			// Undo - Redo.
			if (io.KeysDown[io.KeyMap[ImGuiKey_Z]])
			{
				if (io.KeysDownDuration[io.KeyMap[ImGuiKey_Z]] == 0.0f)
				{
					if (io.KeyCtrl)
					{
						if (io.KeyShift)
						{
							ActionManager::GetInstance()->Redo();
						}
						else
						{
							ActionManager::GetInstance()->Undo();
						}
					}
				}
			}
		}

		void UI::ShowNewSceneWindow()
		{
			if (!m_showNewSceneWindow)
			{
				return;
			}

			ImGui::OpenPopup("NewScene");
			if (ImGui::BeginPopupModal("NewScene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				static String sceneName;
				ImGui::InputTextWithHint("Name", "NewScene", &sceneName);

				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					g_app->OnNewScene(sceneName);
					m_showNewSceneWindow = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::SetItemDefaultFocus();
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
				{
					m_showNewSceneWindow = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

		bool UI::ToggleButton(ImTextureID user_texture_id, const ImVec2& size, bool pushState)
		{
			ImGuiStyle& style = ImGui::GetStyle();
			if (pushState)
			{
				ImGui::PushID(1);
				ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonHovered]);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_ButtonHovered]);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.Colors[ImGuiCol_ButtonHovered]);
			}

			bool newPushState = pushState;
			if (ImGui::ImageButton((void*)(intptr_t)user_texture_id, size))
			{
				newPushState = !pushState; // If pressed toggle.
			}

			if (pushState)
			{
				ImGui::PopStyleColor(3);
				ImGui::PopID();
			}

			return newPushState;
		}

		bool UI::ToggleButton(const String& text, const ImVec2& size, bool pushState)
		{
			ImGuiStyle& style = ImGui::GetStyle();
			if (pushState)
			{
				ImGui::PushID(1);
				ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonHovered]);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_ButtonHovered]);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.Colors[ImGuiCol_ButtonHovered]);
			}

			bool newPushState = pushState;
			if (ImGui::Button(text.c_str(), size))
			{
				newPushState = !pushState; // If pressed toggle.
			}

			if (pushState)
			{
				ImGui::PopStyleColor(3);
				ImGui::PopID();
			}

			return newPushState;
		}

		Window::Window()
		{
		}

		Window::~Window()
		{
		}

		void Window::SetVisibility(bool visible)
		{
			m_visible = visible;
		}

		bool Window::IsActive() const
		{
			return m_active;
		}

		bool Window::IsVisible() const
		{
			return m_visible;
		}

		bool Window::MouseHovers() const
		{
			return m_mouseHover;
		}

		bool Window::CanDispatchEvents() const
		{
			return m_active & m_visible & m_mouseHover;
		}

		void Window::HandleStates()
		{
			ImGui::GetIO().WantCaptureMouse = true;

			m_mouseHover = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup);
			bool rightClick = ImGui::IsMouseDown(ImGuiMouseButton_Right);
			bool leftClick = ImGui::IsMouseDown(ImGuiMouseButton_Left);
			bool middleClick = ImGui::IsMouseDown(ImGuiMouseButton_Middle);

			if ((rightClick || leftClick || middleClick) && m_mouseHover) // Activate with any click.
			{
				if (!m_active)
				{
					ImGui::SetWindowFocus();
					m_active = true;
				}
			}

			if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
			{
				if (m_active)
				{
					m_active = false;
				}
			}
		}

		void Window::SetActive()
		{
			m_active = true;
			ImGui::SetWindowFocus();
		}

		StringInputWindow::StringInputWindow()
		{
			m_visible = false;
		}

		void StringInputWindow::Show()
		{
			if (!m_visible)
			{
				return;
			}

			ImGui::OpenPopup(m_name.c_str());
			if (ImGui::BeginPopupModal(m_name.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::InputTextWithHint(m_inputText.c_str(), m_hint.c_str(), &m_inputVal);

				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					m_visible = false;
					m_taskFn(m_inputVal);
					m_inputVal.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SetItemDefaultFocus();
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
				{
					m_visible = false;
					m_inputVal.clear();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

		YesNoWindow::YesNoWindow(const String& name)
		{
			m_name = name;
		}

		void YesNoWindow::Show()
		{
			if (!m_visible)
			{
				return;
			}

			ImGui::OpenPopup(m_name.c_str());
			if (ImGui::BeginPopupModal(m_name.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				if (ImGui::Button("Yes", ImVec2(120, 0)))
				{
					m_visible = false;
					m_yesCallback();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SetItemDefaultFocus();
				ImGui::SameLine();
				if (ImGui::Button("No", ImVec2(120, 0)))
				{
					m_visible = false;
					m_noCallback();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

	}
}
