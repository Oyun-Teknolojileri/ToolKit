
#include "UI.h"

#include <vector>
#include <algorithm>
#include <unordered_map>

#include "App.h"
#include "EditorViewport.h"
#include "SDL.h"
#include "GlobalDef.h"
#include "Mod.h"
#include "ConsoleWindow.h"
#include "FolderWindow.h"
#include "OverlayUI.h"
#include "OutlinerWindow.h"
#include "PropInspector.h"
#include "Util.h"
#include "PluginWindow.h"
#include "ImGui/imgui_stdlib.h"
#include "Imgui/imgui_impl_sdl.h"
#include "Imgui/imgui_impl_opengl3.h"

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
    std::vector<Window*> UI::m_volatileWindows;
    uint Window::m_baseId = 0;  // unused id.
    std::vector<std::function<void()>> UI::m_postponedActions;

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
    TexturePtr UI::m_playIcon;
    TexturePtr UI::m_pauseIcon;
    TexturePtr UI::m_stopIcon;
    TexturePtr UI::m_vsCodeIcon;
    TexturePtr UI::m_collectionIcon;
    TexturePtr UI::m_arrowsIcon;
    TexturePtr UI::m_lockIcon;
    TexturePtr UI::m_visibleIcon;
    TexturePtr UI::m_invisibleIcon;
    TexturePtr UI::m_lockedIcon;
    TexturePtr UI::m_unlockedIcon;
    TexturePtr UI::m_viewZoomIcon;
    TexturePtr UI::m_gridIcon;

    void UI::Init()
    {
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();

      ImGuiIO& io = ImGui::GetIO();
      io.ConfigFlags |= ImGuiConfigFlags_DockingEnable
      | ImGuiConfigFlags_ViewportsEnable;
      io.ConfigWindowsMoveFromTitleBarOnly = true;

      // Handle font loading.
      static const ImWchar utf8TR[] =
      {
        0x0020, 0x00FF,
        0x00c7, 0x00c7,
        0x00e7, 0x00e7,
        0x011e, 0x011e,
        0x011f, 0x011f,
        0x0130, 0x0130,
        0x0131, 0x0131,
        0x00d6, 0x00d6,
        0x00f6, 0x00f6,
        0x015e, 0x015e,
        0x015f, 0x015f,
        0x00dc, 0x00dc,
        0x00fc, 0x00fc,
        0
      };
      io.Fonts->AddFontFromFileTTF
      (
        FontPath("bmonofont-i18n.ttf").c_str(), 16, nullptr, utf8TR
      );

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

    void UI::ShowDock()
    {
      bool optFullScreen = true;
      static ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_None;

      ImGuiWindowFlags wndFlags = ImGuiWindowFlags_MenuBar
      | ImGuiWindowFlags_NoDocking;
      if (optFullScreen)
      {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        wndFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        wndFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoNavFocus;
      }

      if (dockFlags & ImGuiDockNodeFlags_PassthruCentralNode)
      {
        wndFlags |= ImGuiWindowFlags_NoBackground;
      }

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
      ImGui::Begin("MainDock", nullptr, wndFlags);
      ImGui::PopStyleVar();

      if (optFullScreen)
      {
        ImGui::PopStyleVar(2);
      }

      // DockSpace
      ImGuiIO& io = ImGui::GetIO();
      if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
      {
        ImGuiID dockId = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockId, ImVec2(0.0f, 0.0f), dockFlags);
      }

      ImGui::End();
    }

    void UI::InitIcons()
    {
      m_selectIcn = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/select.png", true)
      );
      m_selectIcn->Init();
      m_cursorIcn = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/cursor.png", true)
      );
      m_cursorIcn->Init();
      m_moveIcn = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/move.png", true)
      );
      m_moveIcn->Init();
      m_rotateIcn = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/rotate.png", true)
      );
      m_rotateIcn->Init();
      m_scaleIcn = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/scale.png", true)
      );
      m_scaleIcn->Init();
      m_snapIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/snap.png", true)
      );
      m_snapIcon->Init();
      m_audioIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/audio.png", true)
      );
      m_audioIcon->Init();
      m_cameraIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/camera.png", true)
      );
      m_cameraIcon->Init();
      m_clipIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/clip.png", true)
      );
      m_clipIcon->Init();
      m_fileIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/file.png", true)
      );
      m_fileIcon->Init();
      m_folderIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/folder.png", true)
      );
      m_folderIcon->Init();
      m_imageIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/image.png", true)
      );
      m_imageIcon->Init();
      m_lightIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/light.png", true)
      );
      m_lightIcon->Init();
      m_materialIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/material.png", true)
      );
      m_materialIcon->Init();
      m_meshIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/mesh.png", true)
      );
      m_meshIcon->Init();
      m_armatureIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/armature.png", true)
      );
      m_armatureIcon->Init();
      m_codeIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/code.png", true)
      );
      m_codeIcon->Init();
      m_boneIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/bone.png", true)
      );
      m_boneIcon->Init();
      m_worldIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/world.png", true)
      );
      m_worldIcon->Init();
      m_axisIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/axis.png", true)
      );
      m_axisIcon->Init();
      m_playIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/play.png", true)
      );
      m_playIcon->Init();
      m_pauseIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/pause.png", true)
      );
      m_pauseIcon->Init();
      m_stopIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/stop.png", true)
      );
      m_stopIcon->Init();
      m_vsCodeIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/vscode.png", true)
      );
      m_vsCodeIcon->Init();
      m_collectionIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/collection.png", true)
      );
      m_collectionIcon->Init();
      m_arrowsIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/arrows.png", true)
      );
      m_arrowsIcon->Init();
      m_lockIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/locked.png", true)
      );
      m_lockIcon->Init();
      m_visibleIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/visible.png", true)
      );
      m_visibleIcon->Init();
      m_invisibleIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/invisible.png", true)
      );
      m_invisibleIcon->Init();
      m_lockedIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/small_locked.png", true)
      );
      m_lockedIcon->Init();
      m_unlockedIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/small_unlocked.png", true)
      );
      m_unlockedIcon->Init();
      m_viewZoomIcon = GetTextureManager()->Create<Texture>
      (
        TexturePath("Icons/viewzoom.png", true)
      );
      m_viewZoomIcon->Init();
      m_gridIcon = GetTextureManager()->Create<Texture>
        (
        TexturePath("Icons/grid.png", true)
        );
      m_gridIcon->Init();
    }

    void UI::InitTheme()
    {
      ImGui::SetColorEditOptions
      (
        ImGuiColorEditFlags_PickerHueWheel
        | ImGuiColorEditFlags_NoOptions
      );

      ImGuiStyle* style = &ImGui::GetStyle();
      style->WindowRounding = 5.3f;
      style->GrabRounding = style->FrameRounding = 2.3f;
      style->ScrollbarRounding = 5.0f;
      style->ItemSpacing.y = 6.5f;
      style->ColorButtonPosition = ImGuiDir_Left;

      // style->WindowPadding = ImVec2(2.0f, 2.0f);
      style->WindowBorderSize = 0.0f;
      style->ChildBorderSize = 0.0f;
      style->PopupBorderSize = 0.0f;
      style->FrameBorderSize = 0.0f;
      style->TabBorderSize = 0.0f;
      style->PopupRounding = 0.0f;
      style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
      style->WindowMenuButtonPosition = ImGuiDir_Right;

      style->Colors[ImGuiCol_Text] =
      {
        0.73333335f,
        0.73333335f,
        0.73333335f,
        1.00f
      };
      style->Colors[ImGuiCol_TextDisabled] =
      {
        0.34509805f,
        0.34509805f,
        0.34509805f,
        1.00f
      };
      style->Colors[ImGuiCol_WindowBg] =
      {
        0.23529413f,
        0.24705884f,
        0.25490198f,
        0.94f
      };
      style->Colors[ImGuiCol_ChildBg] =
      {
        0.23529413f,
        0.24705884f,
        0.25490198f,
        0.00f
      };
      style->Colors[ImGuiCol_PopupBg] = { 0.13f, 0.13f, 0.13f, 0.94f };
      style->Colors[ImGuiCol_Border] =
      {
        0.33333334f,
        0.33333334f,
        0.33333334f,
        0.50f
      };
      style->Colors[ImGuiCol_BorderShadow] =
      {
        0.15686275f,
        0.15686275f,
        0.15686275f,
        0.00f
      };
      style->Colors[ImGuiCol_FrameBg] =
      {
        0.16862746f,
        0.16862746f,
        0.16862746f,
        0.54f
      };
      style->Colors[ImGuiCol_FrameBgHovered] =
      {
        0.453125f,
        0.67578125f,
        0.99609375f,
        0.67f
      };
      style->Colors[ImGuiCol_FrameBgActive] =
      {
        0.47058827f,
        0.47058827f,
        0.47058827f,
        0.67f
      };
      style->Colors[ImGuiCol_TitleBg] = { 0.04f, 0.04f, 0.04f, 1.00f };
      style->Colors[ImGuiCol_TitleBgCollapsed] =
      {
        0.16f,
        0.29f,
        0.48f,
        1.00f
      };
      style->Colors[ImGuiCol_TitleBgActive] = { 0.00f, 0.00f, 0.00f, 0.51f };
      style->Colors[ImGuiCol_MenuBarBg] =
      {
        0.27058825f,
        0.28627452f,
        0.2901961f,
        0.80f
      };
      style->Colors[ImGuiCol_ScrollbarBg] =
      {
        0.27058825f,
        0.28627452f,
        0.2901961f,
        0.60f
      };
      style->Colors[ImGuiCol_ScrollbarGrab] =
      {
        0.21960786f,
        0.30980393f,
        0.41960788f,
        0.51f
      };
      style->Colors[ImGuiCol_ScrollbarGrabHovered] =
      {
        0.21960786f,
        0.30980393f,
        0.41960788f,
        1.00f
      };
      style->Colors[ImGuiCol_ScrollbarGrabActive] =
      {
        0.13725491f,
        0.19215688f,
        0.2627451f,
        0.91f
      };
      // style->Colors[ImGuiCol_ComboBg] = {0.1f, 0.1f, 0.1f, 0.99f};
      style->Colors[ImGuiCol_CheckMark] =
      {
        0.90f,
        0.90f,
        0.90f,
        0.83f
      };
      style->Colors[ImGuiCol_SliderGrab] =
      {
        0.70f,
        0.70f,
        0.70f,
        0.62f
      };
      style->Colors[ImGuiCol_SliderGrabActive] =
      {
        0.30f,
        0.30f,
        0.30f,
        0.84f
      };
      style->Colors[ImGuiCol_Button] =
      {
        0.33333334f,
        0.3529412f,
        0.36078432f,
        0.49f
      };
      style->Colors[ImGuiCol_ButtonHovered] =
      {
        0.21960786f,
        0.30980393f,
        0.41960788f,
        1.00f
      };
      style->Colors[ImGuiCol_ButtonActive] =
      {
        0.13725491f,
        0.19215688f,
        0.2627451f,
        1.00f
      };
      style->Colors[ImGuiCol_Header] =
      {
        0.33333334f,
        0.3529412f,
        0.36078432f,
        0.53f
      };
      style->Colors[ImGuiCol_HeaderHovered] =
      {
        0.453125f,
        0.67578125f,
        0.99609375f,
        0.67f
      };
      style->Colors[ImGuiCol_HeaderActive] =
      {
        0.47058827f,
        0.47058827f,
        0.47058827f,
        0.67f
      };
      style->Colors[ImGuiCol_Separator] =
      {
        0.31640625f,
        0.31640625f,
        0.31640625f,
        1.00f
      };
      style->Colors[ImGuiCol_SeparatorHovered] =
      {
        0.31640625f,
        0.31640625f,
        0.31640625f,
        1.00f
      };
      style->Colors[ImGuiCol_SeparatorActive] =
      {
        0.31640625f,
        0.31640625f,
        0.31640625f,
        1.00f
      };
      style->Colors[ImGuiCol_ResizeGrip] =
      {
        1.00f,
        1.00f,
        1.00f,
        0.85f
      };
      style->Colors[ImGuiCol_ResizeGripHovered] =
      {
        1.00f,
        1.00f,
        1.00f,
        0.60f
      };
      style->Colors[ImGuiCol_ResizeGripActive] =
      {
        1.00f,
        1.00f,
        1.00f,
        0.90f
      };
      style->Colors[ImGuiCol_PlotLines] =
      {
        0.61f,
        0.61f,
        0.61f,
        1.00f
      };
      style->Colors[ImGuiCol_PlotLinesHovered] =
      {
        1.00f,
        0.43f,
        0.35f,
        1.00f
      };
      style->Colors[ImGuiCol_PlotHistogram] =
      {
        0.90f,
        0.70f,
        0.00f,
        1.00f
      };
      style->Colors[ImGuiCol_PlotHistogramHovered] =
      {
        1.00f,
        0.60f,
        0.00f,
        1.00f
      };
      style->Colors[ImGuiCol_TextSelectedBg] =
      {
        0.18431373f,
        0.39607847f,
        0.79215693f,
        0.90f
      };
    }

    void UI::InitSettings()
    {
      String path = "./imgui.ini";
      if (CheckFile(path))
      {
        ImGui::LoadIniSettingsFromDisk(path.c_str());
      }
      else
      {
        path = ConcatPaths({ DefaultPath(), "defaultUI.ini" });
        if (CheckFile(path))
        {
          ImGui::LoadIniSettingsFromDisk(path.c_str());
        }
      }
    }

    void UI::ShowUI()
    {
      ShowDock();
      ShowAppMainMenuBar();

      for (Window* wnd : g_app->m_windows)
      {
        if (wnd->IsVisible())
        {
          wnd->Show();
        }
      }

      if (g_app->m_playWindow->IsVisible())
      {
        g_app->m_playWindow->Show();
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

      // Show & Destroy if not visible.
      for (int i = static_cast<int>(m_volatileWindows.size()) - 1; i > -1; i--)
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

        // Always serve the last popup. Imgui popups are modal.
        // This break gives us the ability to serve last arriving modal.
        break;
      }
    }

    void UI::BeginUI()
    {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame(g_window);
      ImGui::NewFrame();
    }

    void UI::EndUI()
    {
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      ImGui::EndFrame();

      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      SDL_GL_MakeCurrent(g_window, g_context);

      // UI deferred functions.
      for (auto& action : m_postponedActions)
      {
        action();
      }
      m_postponedActions.clear();
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

        if (ImGui::BeginMenu("Projects"))
        {
          ShowMenuProjects();
          ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
      }
    }

    void UI::ShowMenuFile()
    {
      if (ImGui::BeginMenu("Scene"))
      {
        if (ImGui::MenuItem("New"))
        {
          StringInputWindow* inputWnd =
          new StringInputWindow("NewScene##NwScn1", true);
          inputWnd->m_inputVal = g_newSceneStr;
          inputWnd->m_inputLabel = "Name";
          inputWnd->m_hint = "Scene name";
          inputWnd->m_taskFn = [](const String& val)
          {
            g_app->OnNewScene(val);
          };
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Save", "Ctrl+S"))
        {
          g_app->OnSaveScene();
        }

        if (ImGui::MenuItem("SaveAs"))
        {
          g_app->OnSaveAsScene();
        }
        ImGui::EndMenu();
      }

      if (ImGui::MenuItem("Quit", "Alt+F4"))
      {
        g_app->OnQuit();
      }

      ImGui::Separator();
      ImGui::Text("%s", TKVersionStr);
    }

    void UI::ShowMenuWindows()
    {
      auto handleMultiWindowFn = [](Window::Type windowType) -> void
      {
        for
        (
          int i = static_cast<int>(g_app->m_windows.size()) - 1;
          i >= 0;
          i--
        )
        {
          Window* wnd = g_app->m_windows[i];
          if (wnd->GetType() != windowType)
          {
            continue;
          }

          String sId = "menuId" + std::to_string(i);
          ImGui::PushID(sId.c_str());
          ImGui::BeginGroup();
          bool vis = wnd->IsVisible();
          if (ImGui::Checkbox(wnd->m_name.c_str(), &vis))
          {
            wnd->SetVisibility(vis);
          }

          float width = ImGui::CalcItemWidth();
          width -= 50;

          ImGui::SameLine(width);
          if (ImGui::Button("x"))
          {
            g_app->m_windows.erase(g_app->m_windows.begin() + i);
            SafeDel(wnd);
          }
          ImGui::EndGroup();
          ImGui::PopID();
        }
      };

      if (ImGui::BeginMenu("Viewport"))
      {
        handleMultiWindowFn(Window::Type::Viewport);
        handleMultiWindowFn(Window::Type::Viewport2d);

        if (ImGui::MenuItem("Add Viewport", "Alt+V"))
        {
          EditorViewport* vp = new EditorViewport(640, 480);
          g_app->m_windows.push_back(vp);
        }
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Resource Window"))
      {
        handleMultiWindowFn(Window::Type::Browser);

        if (ImGui::MenuItem("Add Browser", "Alt+B"))
        {
          FolderWindow* wnd = new FolderWindow();
          wnd->m_name = g_assetBrowserStr + " " + std::to_string(wnd->m_id);
          wnd->Iterate(ResourcePath(), true);
          g_app->m_windows.push_back(wnd);
        }

        ImGui::EndMenu();
      }

      ImGui::Separator();

      if
      (
        ImGui::MenuItem
        (
          "Console Window",
          "Alt+C",
          nullptr,
          !g_app->GetConsole()->IsVisible()
        )
      )
      {
        g_app->GetConsole()->SetVisibility(true);
      }

      if
      (
        ImGui::MenuItem
        (
          "Outliner Window",
          "Alt+O",
          nullptr,
          !g_app->GetOutliner()->IsVisible()
        )
      )
      {
        g_app->GetOutliner()->SetVisibility(true);
      }

      if
      (
        ImGui::MenuItem
        (
          "Property Inspector",
          "Alt+P",
          nullptr,
          !g_app->GetPropInspector()->IsVisible()
        )
      )
      {
        g_app->GetPropInspector()->SetVisibility(true);
      }

      if
      (
        ImGui::MenuItem
        (
          "Material Inspector",
          "Alt+R",
          nullptr,
          !g_app->GetMaterialInspector()->IsVisible()
        )
      )
      {
        g_app->GetMaterialInspector()->SetVisibility(true);
      }

      if (PluginWindow* wnd = g_app->GetWindow<PluginWindow>("Plugin"))
      {
        if (ImGui::MenuItem("Plugin Window", "", nullptr, !wnd->IsVisible()))
        {
          wnd->SetVisibility(true);
        }
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Reset Layout"))
      {
        m_postponedActions.push_back
        (
          []() -> void
          {
            g_app->ResetUI();
          }
        );
      }

#ifdef TK_DEBUG
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
#endif
    }

    void UI::ShowMenuProjects()
    {
      if (ImGui::MenuItem("New Project"))
      {
        StringInputWindow* inputWnd =
        new StringInputWindow("NewProject", true);
        inputWnd->m_inputVal = "New Project";
        inputWnd->m_inputLabel = "Name";
        inputWnd->m_hint = "Project name";
        inputWnd->m_taskFn = [](const String& val)
        {
          g_app->OnNewProject(val);
        };
      }

      if (ImGui::BeginMenu("Open Project"))
      {
        for (const Project& project : g_app->m_workspace.m_projects)
        {
          if (ImGui::MenuItem(project.name.c_str()))
          {
            g_app->OpenProject(project);
          }
        }
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Publish"))
      {
        if (ImGui::MenuItem("Web"))
        {
          g_app->m_publishManager->Publish(PublishPlatform::Web);
        }

        ImGui::EndMenu();
      }

      /*
      for (const Project& project : g_app->m_workspace.m_projects)
      {
        if (ImGui::BeginMenu(project.name.c_str()))
        {
          if (ImGui::MenuItem("Open"))
          {
            g_app->OpenProject(project);
          }
          
          if (ImGui::BeginMenu("Publish"))
          {
            if (ImGui::MenuItem("Web"))
            {
              // TODO go on
            }

            ImGui::EndMenu();
          }

          ImGui::EndMenu();
        }
      }
      */
    }

    void UI::ShowImportWindow()
    {
      static String load;
      if (!ImportData.showImportWindow || ImportData.files.empty())
      {
        load.clear();
        return;
      }

      if (load.empty())
      {
        std::fstream importList;
        load = ConcatPaths({ "..", "Utils", "Import", "importList.txt" });
        importList.open(load, std::ios::out);
        if (importList.is_open())
        {
          for (String& file : ImportData.files)
          {
            if (g_app->CanImport(file))
            {
              importList << file + "\n";
            }
          }
        }
      }

      if (g_app->m_importSlient)
      {
        g_app->Import(load, ImportData.subDir, ImportData.overwrite);
        load.clear();
        ImportData.files.clear();
        ImportData.showImportWindow = false;
        return;
      }

      ImGui::OpenPopup("Import");
      if
      (
        ImGui::BeginPopupModal
        (
          "Import",
          NULL,
          ImGuiWindowFlags_AlwaysAutoResize
        )
      )
      {
        ImGui::Text("Import file: \n\n");
        for (size_t i = 0; i < ImportData.files.size(); i++)
        {
          ImGui::Text("%s", ImportData.files[i].c_str());
        }
        ImGui::Separator();

        static StringArray fails;
        if (!ImportData.files.empty() && fails.empty())
        {
          for
          (
            int i = static_cast<int>(ImportData.files.size()) - 1;
            i >= 0;
            i--
          )
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
          ImGui::Text
          (
            "Fallowing imports failed due to:\nFile format is"
            " not supported.\nSuported formats are fbx, glb, obj."
          );
          for (String& file : fails)
          {
            ImGui::Text("%s", file.c_str());
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
          if
          (
            g_app->Import(load, ImportData.subDir, ImportData.overwrite) == -1
          )
          {
            // Fall back to search.
            ImGui::EndPopup();
            ImportData.showImportWindow = false;
            return;
          }

          load.clear();
          fails.clear();
          ImportData.files.clear();
          ImportData.showImportWindow = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
          load.clear();
          fails.clear();
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
      if
      (
        ImGui::BeginPopupModal
        (
          "SearchFile",
          NULL,
          ImGuiWindowFlags_AlwaysAutoResize
        )
      )
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
          ImGui::Text("%s", s->c_str());
        }

        int itemCnt = static_cast<int>(SearchFileData.searchPaths.size());
        const char** items = new const char* [itemCnt];
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
            SearchFileData.searchPaths.erase
            (
              SearchFileData.searchPaths.begin() + currItem
            );
          }
        }

        if (ImGui::Button("Search", ImVec2(120, 0)))
        {
          g_app->Import("", ImportData.subDir, ImportData.overwrite);
          ImportData.files.clear();
          ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Abort", ImVec2(120, 0)))
        {
          ImportData.files.clear();
          SearchFileData.missingFiles.clear();
          SearchFileData.showSearchFileWindow = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
    }

    void UI::HelpMarker(const String& key, const char* desc, float wait)
    {
      static std::unordered_map<String, float> helpTimers;
      if (helpTimers.find(key) == helpTimers.end())
      {
        helpTimers[key] = 0.0f;
      }
      float* elapsedHoverTime = &helpTimers[key];

      if (ImGui::IsItemHovered())
      {
        *elapsedHoverTime += ImGui::GetIO().DeltaTime;
        if (wait > *elapsedHoverTime)
        {
          return;
        }

        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
      }
      else if (*elapsedHoverTime > 0.0f)
      {
        *elapsedHoverTime = 0.0f;
      }
    }

    void UI::ShowNewSceneWindow()
    {
      if (!m_showNewSceneWindow)
      {
        return;
      }

      ImGui::OpenPopup(g_newSceneStr.c_str());
      if
      (
        ImGui::BeginPopupModal
        (
          g_newSceneStr.c_str(),
          NULL,
          ImGuiWindowFlags_AlwaysAutoResize
        )
      )
      {
        static String sceneName;

        ImGui::InputTextWithHint("Name", g_newSceneStr.c_str(), &sceneName);
        ImGui::SetItemDefaultFocus();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
          g_app->OnNewScene(sceneName);
          m_showNewSceneWindow = false;
          ImGui::CloseCurrentPopup();
        }
        // ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
          m_showNewSceneWindow = false;
          ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
      }
    }

    bool UI::ImageButtonDecorless
    (
      uint textureID,
      const Vec2& size,
      bool flipImage
    )
    {
      ImGui::PushStyleColor(ImGuiCol_Button, Vec4());
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Vec4());
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, Vec4());
      ImVec2 texCoords = flipImage ? ImVec2(1.0f, -1.0f) : ImVec2(1.0f, 1.0f);
      bool res = ImGui::ImageButton
      (
        reinterpret_cast<void*>((intptr_t)textureID),
        size,
        ImVec2(0.0f, 0.0f),
        texCoords
      );
      ImGui::PopStyleColor(3);

      return res;
    }

    bool UI::ToggleButton(uint textureID, const Vec2& size, bool pushState)
    {
      ImGuiStyle& style = ImGui::GetStyle();
      if (pushState)
      {
        ImGui::PushID(1);
        ImGui::PushStyleColor
        (
          ImGuiCol_Button,
          style.Colors[ImGuiCol_ButtonHovered]
        );
        ImGui::PushStyleColor
        (
          ImGuiCol_ButtonHovered,
          style.Colors[ImGuiCol_ButtonHovered]
        );
        ImGui::PushStyleColor
        (
          ImGuiCol_ButtonActive,
          style.Colors[ImGuiCol_ButtonHovered]
        );
      }

      bool newPushState = pushState;
      if
      (
        ImGui::ImageButton
        (
          reinterpret_cast<void*>((intptr_t)textureID),
          size
        )
      )
      {
        newPushState = !pushState;  // If pressed toggle.
      }

      if (pushState)
      {
        ImGui::PopStyleColor(3);
        ImGui::PopID();
      }

      return newPushState;
    }

    bool UI::ToggleButton(const String& text, const Vec2& size, bool pushState)
    {
      ImGuiStyle& style = ImGui::GetStyle();
      if (pushState)
      {
        ImGui::PushID(1);
        ImGui::PushStyleColor
        (
          ImGuiCol_Button,
          style.Colors[ImGuiCol_ButtonHovered]
        );
        ImGui::PushStyleColor
        (
          ImGuiCol_ButtonHovered,
          style.Colors[ImGuiCol_ButtonHovered]
        );
        ImGui::PushStyleColor
        (
          ImGuiCol_ButtonActive,
          style.Colors[ImGuiCol_ButtonHovered]
        );
      }

      bool newPushState = pushState;
      if (ImGui::Button(text.c_str(), size))
      {
        newPushState = !pushState;  // If pressed toggle.
      }

      if (pushState)
      {
        ImGui::PopStyleColor(3);
        ImGui::PopID();
      }

      return newPushState;
    }

    float g_centeredTextOffset = 0.0f;
    bool UI::BeginCenteredTextButton(const String& text, const String& id)
    {
      assert
      (
        g_centeredTextOffset == 0.0f &&
        "Begin / End CenteredTextButton mismatch !"
      );

      Vec2 min = ImGui::GetWindowContentRegionMin();
      Vec2 max = ImGui::GetWindowContentRegionMax();
      Vec2 size = max - min;

      ImGui::AlignTextToFramePadding();
      ImVec2 tSize = ImGui::CalcTextSize(text.c_str());
      g_centeredTextOffset = (size.x - tSize.x) * 0.5f;
      ImGui::Indent(g_centeredTextOffset);

      String buttonText = text;
      if (!id.empty())
      {
        buttonText += "##" + id;
      }

      return ImGui::Button(buttonText.c_str());
    }

    void UI::EndCenteredTextButton()
    {
      ImGui::Indent(-g_centeredTextOffset);
      g_centeredTextOffset = 0.0f;
    }

    Window::Window()
    {
      m_id = ++m_baseId;
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

    bool Window::CanDispatchSignals() const
    {
      return m_active && m_visible && m_mouseHover;
    }

    void Window::DispatchSignals() const
    {
    }

    void Window::Serialize(XmlDocument* doc, XmlNode* parent) const
    {
      XmlNode* node = doc->allocate_node(rapidxml::node_element, "Window");
      if (parent != nullptr)
      {
        parent->append_node(node);
      }
      else
      {
        doc->append_node(node);
      }

      WriteAttr(node, doc, "name", m_name);
      WriteAttr(node, doc, "id", std::to_string(m_id));
      WriteAttr
      (
        node,
        doc,
        "type",
        std::to_string(static_cast<int>(GetType()))
      );
      WriteAttr
      (
        node,
        doc,
        "visible",
        std::to_string(static_cast<int>(m_visible))
      );
    }

    void Window::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      XmlNode* node = nullptr;
      if (parent != nullptr)
      {
        node = parent;
      }
      else
      {
        node = doc->first_node("Window");
      }

      ReadAttr(node, "name", m_name);
      ReadAttr(node, "id", m_id);
      // Type is determined by the corrsesponding constructor.
      ReadAttr(node, "visible", m_visible);
    }

    void Window::HandleStates()
    {
      ImGui::GetIO().WantCaptureMouse = true;

      m_mouseHover = ImGui::IsWindowHovered
      (
        ImGuiHoveredFlags_RootAndChildWindows
        | ImGuiHoveredFlags_AllowWhenBlockedByPopup
      );
      bool rightClick = ImGui::IsMouseDown(ImGuiMouseButton_Right);
      bool leftClick = ImGui::IsMouseDown(ImGuiMouseButton_Left);
      bool middleClick = ImGui::IsMouseDown(ImGuiMouseButton_Middle);

      // Activate with any click.
      if ((rightClick || leftClick || middleClick) && m_mouseHover)
      {
        if
          (
            ImGui::IsMouseDragging(ImGuiMouseButton_Left) ||
            ImGui::IsMouseDragging(ImGuiMouseButton_Right) ||
            ImGui::IsMouseDragging(ImGuiMouseButton_Middle)
            )
        {
          return;
        }

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

    void Window::ModShortCutSignals(const IntArray& mask) const
    {
      if (!CanDispatchSignals())
      {
        return;
      }

      if
        (
          ImGui::IsKeyPressed(ImGuiKey_Delete, false) &&
          !Exist(mask, ImGuiKey_Delete)
          )
      {
        ModManager::GetInstance()->DispatchSignal(BaseMod::m_delete);
      }

      if
        (
          (
            ImGui::IsKeyDown(ImGuiKey_ModCtrl) ||
            ImGui::IsKeyDown(ImGuiKey_ModShift)
            ) &&
          ImGui::IsKeyPressed(ImGuiKey_D, false) &&
          !ImGui::IsMouseDown(ImGuiMouseButton_Right) &&
          !Exist(mask, ImGuiKey_D)
          )
      {
        ModManager::GetInstance()->DispatchSignal(BaseMod::m_duplicate);
      }

      if
        (
          ImGui::IsKeyPressed(ImGuiKey_B, false) &&
          !Exist(mask, ImGuiKey_B)
          )
      {
        ModManager::GetInstance()->SetMod(true, ModId::Select);
      }

      if
        (
          ImGui::IsKeyPressed(ImGuiKey_S, false) &&
          !ImGui::IsMouseDown(ImGuiMouseButton_Right) &&
          !Exist(mask, ImGuiKey_S)
          )
      {
        ModManager::GetInstance()->SetMod(true, ModId::Scale);
      }

      if
        (
          ImGui::IsKeyPressed(ImGuiKey_R, false) &&
          !Exist(mask, ImGuiKey_R)
          )
      {
        ModManager::GetInstance()->SetMod(true, ModId::Rotate);
      }

      if
        (
          ImGui::IsKeyPressed(ImGuiKey_G, false) &&
          !Exist(mask, ImGuiKey_G)
          )
      {
        ModManager::GetInstance()->SetMod(true, ModId::Move);
      }

      EditorScenePtr currSecne = g_app->GetCurrentScene();
      if
        (
          ImGui::IsKeyDown(ImGuiKey_ModCtrl) &&
          ImGui::IsKeyPressed(ImGuiKey_S, false) &&
          !Exist(mask, ImGuiKey_S)
          )
      {
        XmlDocument* doc = new XmlDocument();
        currSecne->Serialize(doc, nullptr);
        SafeDel(doc);
      }

      if
        (
          ImGui::IsKeyPressed(ImGuiKey_F, false) &&
          !Exist(mask, ImGuiKey_F)
          )
      {
        if (Window* wnd = g_app->GetOutliner())
        {
          if (Entity* ntt = currSecne->GetCurrentSelection())
          {
            OutlinerWindow* outliner = static_cast<OutlinerWindow*> (wnd);
            outliner->Focus(ntt);
          }
        }
      }

      // Undo - Redo.
      if
        (
          ImGui::IsKeyPressed(ImGuiKey_Z, false) &&
          !Exist(mask, ImGuiKey_Z)
          )
      {
        if (ImGui::IsKeyDown(ImGuiKey_ModCtrl))
        {
          if (ImGui::IsKeyDown(ImGuiKey_ModShift))
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

    StringInputWindow::StringInputWindow(const String& name, bool showCancel)
    {
      m_name = name;
      m_showCancel = showCancel;
      UI::m_volatileWindows.push_back(this);
    }

    void StringInputWindow::Show()
    {
      if (!m_visible)
      {
        return;
      }

      ImGuiIO& io = ImGui::GetIO();
      ImGui::SetNextWindowPos
      (
        ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
        ImGuiCond_Once,
        ImVec2(0.5f, 0.5f)
      );

      ImGui::OpenPopup(m_name.c_str());
      if
      (
        ImGui::BeginPopupModal
        (
          m_name.c_str(),
          NULL,
          ImGuiWindowFlags_AlwaysAutoResize
        )
      )
      {
        if (ImGui::IsWindowAppearing())
        {
          ImGui::SetKeyboardFocusHere();
        }

        ImGui::InputTextWithHint
        (
          m_inputLabel.c_str(),
          m_hint.c_str(),
          &m_inputVal,
          ImGuiInputTextFlags_AutoSelectAll
        );

        // Center buttons.
        ImGui::BeginTable
        (
          "##FilterZoom", m_showCancel ? 4 : 3,
          ImGuiTableFlags_SizingFixedFit
        );

        ImGui::TableSetupColumn
        (
          "##spaceL",
          ImGuiTableColumnFlags_WidthStretch
        );
        ImGui::TableSetupColumn("##ok");
        if (m_showCancel)
        {
          ImGui::TableSetupColumn("##cancel");
        }
        ImGui::TableSetupColumn
        (
          "##spaceR",
          ImGuiTableColumnFlags_WidthStretch
        );

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
          m_visible = false;
          m_taskFn(m_inputVal);
          m_inputVal.clear();
          ImGui::CloseCurrentPopup();
        }

        if (m_showCancel)
        {
          ImGui::TableNextColumn();
          if (ImGui::Button("Cancel", ImVec2(120, 0)))
          {
            m_visible = false;
            m_inputVal.clear();
            ImGui::CloseCurrentPopup();
          }
        }

        ImGui::EndTable();
        ImGui::EndPopup();
      }
    }

    YesNoWindow::YesNoWindow(const String& name, const String& msg)
    {
      m_name = name;
      m_msg = msg;
    }

    YesNoWindow::YesNoWindow
    (
      const String& name,
      const String& yesBtnText,
      const String& noBtnText,
      const String& msg,
      bool showCancel
    )
    {
      m_name = name;
      m_yesText = yesBtnText;
      m_noText = noBtnText;
      m_msg = msg;
      m_showCancel = showCancel;
    }

    void YesNoWindow::Show()
    {
      if (!m_visible)
      {
        return;
      }

      ImGuiIO& io = ImGui::GetIO();
      ImGui::SetNextWindowPos
      (
        ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
        ImGuiCond_Once,
        ImVec2(0.5f, 0.5f)
      );

      ImGui::OpenPopup(m_name.c_str());
      if
      (
        ImGui::BeginPopupModal
        (
          m_name.c_str(),
          NULL,
          ImGuiWindowFlags_AlwaysAutoResize
        )
      )
      {
        if (!m_msg.empty())
        {
          ImGui::Text("%s", m_msg.c_str());
        }

        // Center buttons.
        ImGui::BeginTable
        (
          "##FilterZoom",
          m_showCancel ? 5 : 4,
          ImGuiTableFlags_SizingFixedFit
        );

        ImGui::TableSetupColumn
        (
          "##spaceL",
          ImGuiTableColumnFlags_WidthStretch
        );
        ImGui::TableSetupColumn("##yes");
        ImGui::TableSetupColumn("##no");
        if (m_showCancel)
        {
          ImGui::TableSetupColumn("##cancel");
        }
        ImGui::TableSetupColumn
        (
          "##spaceR",
          ImGuiTableColumnFlags_WidthStretch
        );

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();

        if
        (
          ImGui::Button
          (
            m_yesText.empty() ? "Yes" : m_yesText.c_str(),
            ImVec2(120, 0)
          )
        )
        {
          m_visible = false;
          m_yesCallback();
          ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::TableNextColumn();

        if
        (
          ImGui::Button
          (
            m_noText.empty() ? "No" : m_noText.c_str(),
            ImVec2(120, 0)
          )
        )
        {
          m_visible = false;
          m_noCallback();
          ImGui::CloseCurrentPopup();
        }

        if (m_showCancel)
        {
          ImGui::TableNextColumn();
          if (ImGui::Button("Cancel", ImVec2(120, 0)))
          {
            m_visible = false;
            ImGui::CloseCurrentPopup();
          }
        }

        ImGui::EndTable();
        ImGui::EndPopup();
      }
    }

  }  // namespace Editor
}  // namespace ToolKit
