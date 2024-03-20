/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "UI.h"

#include "AndroidBuildWindow.h"
#include "App.h"
#include "EditorViewport2d.h"
#include "PopupWindows.h"

#include <Audio.h>
#include <GlErrorReporter.h>
#include <GradientSky.h>
#include <ImGui/backends/imgui_impl_opengl3.h>
#include <ImGui/backends/imgui_impl_sdl2.h>
#include <MathUtil.h>
#include <Prefab.h>
#include <SDL.h>
#include <Sky.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {

    const float g_indentSpacing = 6.0f;
    const int g_treeNodeFlags   = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap;

    bool UI::m_windowMenushowMetrics = false;
    bool UI::m_imguiSampleWindow     = false;
    bool UI::m_showNewSceneWindow    = false;
    float UI::m_hoverTimeForHelp     = 1.0f;
    UI::Blocker UI::BlockerData;
    UI::Import UI::ImportData;
    UI::SearchFile UI::SearchFileData;
    AndroidBuildWindow* UI::m_androidBuildWindow;
    WindowRawPtrArray UI::m_volatileWindows;
    std::vector<TempWindow*> UI::m_tempWindows;
    std::vector<TempWindow*> UI::m_removedTempWindows;
    uint Window::m_baseId = 0; // unused id.
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
    TexturePtr UI::m_skyIcon;
    TexturePtr UI::m_closeIcon;
    TexturePtr UI::m_phoneRotateIcon;
    TexturePtr UI::m_studioLightsToggleIcon;
    TexturePtr UI::m_anchorIcn;
    TexturePtr UI::m_prefabIcn;
    TexturePtr UI::m_buildIcn;
    TexturePtr UI::m_addIcon;
    TexturePtr UI::m_sphereIcon;
    TexturePtr UI::m_cubeIcon;
    TexturePtr UI::m_shaderBallIcon;
    TexturePtr UI::m_diskDriveIcon;
    TexturePtr UI::m_packageIcon;
    TexturePtr UI::m_objectDataIcon;
    TexturePtr UI::m_sceneIcon;

    UI::AnchorPresetImages UI::m_anchorPresetIcons;

    ImFont *LiberationSans, *LiberationSansBold, *IconFont;

    void UI::Init()
    {
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();

      ImGuiIO& io                           = ImGui::GetIO();
      io.ConfigFlags                       |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
      io.ConfigWindowsMoveFromTitleBarOnly  = true;

      // Handle font loading.
      static const ImWchar utf8TR[]         = {0x0020, 0x00FF, 0x00c7, 0x00c7, 0x00e7, 0x00e7, 0x011e, 0x011e, 0x011f,
                                               0x011f, 0x0130, 0x0130, 0x0131, 0x0131, 0x00d6, 0x00d6, 0x00f6, 0x00f6,
                                               0x015e, 0x015e, 0x015f, 0x015f, 0x00dc, 0x00dc, 0x00fc, 0x00fc, 0};

      io.Fonts->Clear();
      LiberationSans =
          io.Fonts->AddFontFromFileTTF(FontPath("LiberationSans-Regular.ttf").c_str(), 14.0f, nullptr, utf8TR);

      static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
      ImFontConfig icons_config;
      icons_config.MergeMode = true;
      // icons_config.PixelSnapH = true;
      IconFont =
          io.Fonts->AddFontFromFileTTF(FontPath(FONT_ICON_FILE_NAME_FA).c_str(), 14.0f, &icons_config, icons_ranges);

      LiberationSansBold =
          io.Fonts->AddFontFromFileTTF(FontPath("LiberationSans-Bold.ttf").c_str(), 14.0f, nullptr, utf8TR);

      io.Fonts->Build();

      ImGui_ImplSDL2_InitForOpenGL(g_window, g_context);
      ImGui_ImplOpenGL3_Init("#version 300 es");

      InitIcons();
      InitTheme();

      m_androidBuildWindow = new AndroidBuildWindow();
    }

    void UI::HeaderText(const char* text)
    {
      ImGui::PushFont(LiberationSansBold);
      ImGui::Text(text);
      ImGui::PopFont();
    }

    void UI::PushBoldFont() { ImGui::PushFont(LiberationSansBold); }

    void UI::PopBoldFont() { ImGui::PopFont(); }

    void UI::UnInit()
    {
      delete m_androidBuildWindow;
      m_volatileWindows.clear();
      assert(m_volatileWindows.size() < 10 && "Overflowing danger.");
      m_volatileWindows.clear();

      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplSDL2_Shutdown();
      ImGui::DestroyContext();
    }

    void LightTheme();
    void DarkTheme();
    void GreyTheme();

    void UI::ShowDock()
    {
      bool optFullScreen                  = true;
      static ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_None;

      ImGuiWindowFlags wndFlags           = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
      if (optFullScreen)
      {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        wndFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove;
        wndFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
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

    void LoadIcon(TexturePtr& icon, const char* path)
    {
      icon = GetTextureManager()->Create<Texture>(TexturePath(path, true));
      icon->Init();
    }

    void UI::InitIcons()
    {
      LoadIcon(m_selectIcn, "Icons/select.png");
      LoadIcon(m_cursorIcn, "Icons/cursor.png");
      LoadIcon(m_moveIcn, "Icons/move.png");
      LoadIcon(m_rotateIcn, "Icons/rotate.png");
      LoadIcon(m_scaleIcn, "Icons/scale.png");
      LoadIcon(m_snapIcon, "Icons/snap.png");
      LoadIcon(m_audioIcon, "Icons/audio.png");
      LoadIcon(m_cameraIcon, "Icons/camera.png");
      LoadIcon(m_clipIcon, "Icons/clip.png");
      LoadIcon(m_fileIcon, "Icons/file.png");
      LoadIcon(m_folderIcon, "Icons/folder.png");
      LoadIcon(m_imageIcon, "Icons/image.png");
      LoadIcon(m_lightIcon, "Icons/light.png");
      LoadIcon(m_materialIcon, "Icons/material.png");
      LoadIcon(m_meshIcon, "Icons/mesh.png");
      LoadIcon(m_armatureIcon, "Icons/armature.png");
      LoadIcon(m_codeIcon, "Icons/code.png");
      LoadIcon(m_boneIcon, "Icons/bone.png");
      LoadIcon(m_worldIcon, "Icons/world.png");
      LoadIcon(m_axisIcon, "Icons/axis.png");
      LoadIcon(m_playIcon, "Icons/play.png");
      LoadIcon(m_pauseIcon, "Icons/pause.png");
      LoadIcon(m_stopIcon, "Icons/stop.png");
      LoadIcon(m_vsCodeIcon, "Icons/vscode.png");
      LoadIcon(m_collectionIcon, "Icons/collection.png");
      LoadIcon(m_arrowsIcon, "Icons/empty_arrows.png");
      LoadIcon(m_lockIcon, "Icons/locked.png");
      LoadIcon(m_visibleIcon, "Icons/visible.png");
      LoadIcon(m_invisibleIcon, "Icons/invisible.png");
      LoadIcon(m_lockedIcon, "Icons/small_locked.png");
      LoadIcon(m_unlockedIcon, "Icons/small_unlocked.png");
      LoadIcon(m_viewZoomIcon, "Icons/view_zoom.png");
      LoadIcon(m_gridIcon, "Icons/grid.png");
      LoadIcon(m_skyIcon, "Icons/outliner_data_volume.png");
      LoadIcon(m_closeIcon, "Icons/close.png");
      LoadIcon(m_phoneRotateIcon, "Icons/rotate-icon.png");
      LoadIcon(m_studioLightsToggleIcon, "Icons/studio_lights_toggle.png");
      LoadIcon(m_anchorIcn, "Icons/anchor_move.png");
      LoadIcon(m_prefabIcn, "Icons/scene_data.png");
      LoadIcon(m_buildIcn, "Icons/build.png");
      LoadIcon(m_addIcon, "Icons/add.png");
      LoadIcon(m_sphereIcon, "Icons/sphere.png");
      LoadIcon(m_cubeIcon, "Icons/cube.png");
      LoadIcon(m_shaderBallIcon, "Icons/shader-ball.png");
      LoadIcon(m_diskDriveIcon, "Icons/disk_drive.png");
      LoadIcon(m_packageIcon, "Icons/package.png");
      LoadIcon(m_objectDataIcon, "Icons/object_data.png");
      LoadIcon(m_sceneIcon, "Icons/scene.png");

      for (uint anchorPresentIndx = 0; anchorPresentIndx < AnchorPresetImages::presetCount; anchorPresentIndx++)
      {
        TexturePtr& preset = m_anchorPresetIcons.m_presetImages[anchorPresentIndx];
        preset             = GetTextureManager()->Create<Texture>(
            TexturePath("Icons/Anchor Presets/" + String(m_anchorPresetIcons.m_presetNames[anchorPresentIndx]) + ".png",
                        true));
        preset->Init();
      }
    }

    void SetTheme(Theme theme)
    {
      if (theme == Theme::Dark)
      {
        DarkTheme();
      }
      else if (theme == Theme::Light)
      {
        LightTheme();
      }
      else // if (theme == Theme::Grey)
      {
        GreyTheme();
      }

      // Fix gamma correction
      if (!GetRenderSystem()->IsGammaCorrectionNeeded())
      {
        float gamma = GetEngineSettings().PostProcessing.Gamma;
        for (ImVec4& col : ImGui::GetStyle().Colors)
        {
          col.x = std::powf(col.x, gamma);
          col.y = std::powf(col.y, gamma);
          col.z = std::powf(col.z, gamma);
        }
      }
    }

    void DarkTheme()
    {
      ImGuiStyle& style                        = ImGui::GetStyle();
      ImVec4* stylCols                         = style.Colors;
      stylCols[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
      stylCols[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
      stylCols[ImGuiCol_WindowBg]              = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
      stylCols[ImGuiCol_ChildBg]               = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
      stylCols[ImGuiCol_PopupBg]               = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
      stylCols[ImGuiCol_Border]                = ImVec4(0.23f, 0.23f, 0.20f, 1.00f);
      stylCols[ImGuiCol_BorderShadow]          = ImVec4(0.05f, 0.05f, 0.05f, 0.50f);
      stylCols[ImGuiCol_FrameBg]               = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
      stylCols[ImGuiCol_FrameBgHovered]        = ImVec4(0.38f, 0.38f, 0.38f, 1.0f);
      stylCols[ImGuiCol_FrameBgActive]         = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
      stylCols[ImGuiCol_TitleBg]               = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
      stylCols[ImGuiCol_TitleBgActive]         = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
      stylCols[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.0f, 0.0f, 0.0f, 0.51f);
      stylCols[ImGuiCol_MenuBarBg]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
      stylCols[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
      stylCols[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
      stylCols[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(.41f, .41f, .41f, 1.f);
      stylCols[ImGuiCol_ScrollbarGrabActive]   = ImVec4(.51f, .51f, .51f, 1.f);
      stylCols[ImGuiCol_CheckMark]             = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
      stylCols[ImGuiCol_SliderGrab]            = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
      stylCols[ImGuiCol_SliderGrabActive]      = ImVec4(0.08f, 0.50f, 0.72f, 1.0f);
      stylCols[ImGuiCol_Button]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
      stylCols[ImGuiCol_ButtonHovered]         = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
      stylCols[ImGuiCol_ButtonActive]          = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
      stylCols[ImGuiCol_Header]                = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
      stylCols[ImGuiCol_HeaderHovered]         = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
      stylCols[ImGuiCol_HeaderActive]          = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
      stylCols[ImGuiCol_Separator]             = style.Colors[ImGuiCol_Border];
      stylCols[ImGuiCol_SeparatorHovered]      = ImVec4(0.41f, 0.42f, 0.44f, 1.0f);
      stylCols[ImGuiCol_SeparatorActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
      stylCols[ImGuiCol_ResizeGrip]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
      stylCols[ImGuiCol_ResizeGripHovered]     = ImVec4(0.29f, 0.3f, 0.31f, 0.67f);
      stylCols[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
      stylCols[ImGuiCol_Tab]                   = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
      stylCols[ImGuiCol_TabHovered]            = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
      stylCols[ImGuiCol_TabActive]             = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
      stylCols[ImGuiCol_TabUnfocused]          = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
      stylCols[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.13f, 0.14f, 0.15f, 1.f);
      stylCols[ImGuiCol_DockingPreview]        = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
      stylCols[ImGuiCol_DockingEmptyBg]        = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
      stylCols[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
      stylCols[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.0f);
      stylCols[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
      stylCols[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.f, 0.6f, 0.f, 1.f);
      stylCols[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
      stylCols[ImGuiCol_DragDropTarget]        = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
      stylCols[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
      stylCols[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.f, 1.0f, 0.7f);
      stylCols[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.8f, 0.8f, 0.80f, 0.2f);
      stylCols[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
      style.GrabRounding = style.FrameRounding = 2.3f;
    }

    static __forceinline ImVec4 Inverse(const ImVec4& input)
    {
      return ImVec4(1.0f - input.x, 1.0f - input.y, 1.0f - input.z, 1.0f);
    }

    void LightTheme()
    {
      DarkTheme();
      ImGuiStyle& style = ImGui::GetStyle();
      for (int i = 0; i < ImGuiCol_COUNT; ++i)
      {
        style.Colors[i] = Inverse(style.Colors[i]);
      }
    }

    void GreyTheme()
    {
      ImGuiStyle& style                      = ImGui::GetStyle();
      ImVec4* colors                         = style.Colors;

      colors[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
      colors[ImGuiCol_TextDisabled]          = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
      colors[ImGuiCol_ChildBg]               = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
      colors[ImGuiCol_WindowBg]              = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
      colors[ImGuiCol_PopupBg]               = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
      colors[ImGuiCol_Border]                = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
      colors[ImGuiCol_BorderShadow]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
      colors[ImGuiCol_FrameBg]               = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
      colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
      colors[ImGuiCol_FrameBgActive]         = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
      colors[ImGuiCol_TitleBg]               = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
      colors[ImGuiCol_TitleBgActive]         = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
      colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.17f, 0.17f, 0.17f, 0.9f);
      colors[ImGuiCol_MenuBarBg]             = ImVec4(0.335f, 0.335f, 0.335f, 1.0f);
      colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
      colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
      colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.52f, 0.52f, 0.52f, 1.f);
      colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.76f, 0.76f, 0.76f, 1.0f);
      colors[ImGuiCol_CheckMark]             = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
      colors[ImGuiCol_SliderGrab]            = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
      colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
      colors[ImGuiCol_Button]                = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
      colors[ImGuiCol_ButtonHovered]         = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
      colors[ImGuiCol_ButtonActive]          = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
      colors[ImGuiCol_Header]                = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
      colors[ImGuiCol_HeaderHovered]         = ImVec4(0.47f, 0.47f, 0.47f, 1.0f);
      colors[ImGuiCol_HeaderActive]          = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
      colors[ImGuiCol_Separator]             = ImVec4(0.000f, 0.000f, 0.00f, 0.137f);
      colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.700f, 0.671f, 0.60f, 0.29f);
      colors[ImGuiCol_SeparatorActive]       = ImVec4(0.702f, 0.671f, 0.60f, 0.674f);
      colors[ImGuiCol_ResizeGrip]            = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
      colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
      colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
      colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
      colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.0f);
      colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
      colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.0f, 0.60f, 0.00f, 1.0f);
      colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
      colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
      colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
      colors[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
      colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.70f);
      colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

      style.PopupRounding                    = 3;

#ifdef IMGUI_HAS_DOCK
      style.TabBorderSize                 = 1;
      style.TabRounding                   = 3;

      colors[ImGuiCol_DockingEmptyBg]     = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
      colors[ImGuiCol_Tab]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
      colors[ImGuiCol_TabHovered]         = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
      colors[ImGuiCol_TabActive]          = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
      colors[ImGuiCol_TabUnfocused]       = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
      colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
      colors[ImGuiCol_DockingPreview]     = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);

      if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
      {
        style.WindowRounding              = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
      }
#endif
    }

    void UI::InitTheme()
    {
      ImGui::SetColorEditOptions(ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoOptions);
      SetTheme(Theme::Dark);
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
        path = ConcatPaths({ConfigPath(), g_uiLayoutFile});

        if (CheckFile(path))
        {
          ImGui::LoadIniSettingsFromDisk(path.c_str());
        }
      }
    }

    void UI::AddTempWindow(TempWindow* window) { m_tempWindows.push_back(window); }

    void UI::RemoveTempWindow(TempWindow* window) { m_removedTempWindows.push_back(window); }

    void UI::ShowUI()
    {
      ShowDock();
      ShowAppMainMenuBar();

      for (WindowPtr wnd : g_app->m_windows)
      {
        if (wnd->IsVisible())
        {
          wnd->Show();
        }
      }

      for (auto wnd : m_tempWindows)
      {
        wnd->Show();
      }

      erase_if(m_tempWindows, [&](TempWindow* window) -> bool { return contains(m_removedTempWindows, window); });
      m_removedTempWindows.clear();

      if (g_app->m_simulationWindow->IsVisible())
      {
        g_app->m_simulationWindow->Show();
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
      ShowBlocker();

      // Show & Destroy if not visible.
      for (int i = (int) (m_volatileWindows.size()) - 1; i > -1; i--)
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

        if (ImGui::BeginMenu("Themes"))
        {
          ImGui::Separator();

          if (ImGui::MenuItem("Dark Theme"))
          {
            SetTheme(Theme::Dark);
          }
          if (ImGui::MenuItem("Grey Theme"))
          {
            SetTheme(Theme::Grey);
          }
          if (ImGui::MenuItem("Light Theme"))
          {
            SetTheme(Theme::Light);
          }

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
          StringInputWindow* inputWnd = new StringInputWindow("NewScene##NwScn1", true);
          inputWnd->m_inputVal        = g_newSceneStr;
          inputWnd->m_inputLabel      = "Name";
          inputWnd->m_hint            = "Scene name";
          inputWnd->m_taskFn          = [](const String& val) { g_app->OnNewScene(val); };
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

        if (ImGui::MenuItem("Save All Resources"))
        {
          g_app->SaveAllResources();
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

    template <typename T>
    void ShowPersistentWindow(const String& name)
    {
      if (std::shared_ptr<T> wnd = g_app->CreateOrRetrieveWindow<T>(name))
      {
        if (ImGui::MenuItem(name.c_str(), nullptr, nullptr, !wnd->IsVisible()))
        {
          wnd->SetVisibility(true);
        }
      }
    }

    void UI::ShowMenuWindows()
    {
      auto handleMultiWindowFn = [](ClassMeta* Class) -> void
      {
        for (int i = (int) (g_app->m_windows.size()) - 1; i >= 0; i--)
        {
          WindowPtr wnd = g_app->m_windows[i];
          if (wnd->Class() != Class)
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

          float width  = ImGui::CalcItemWidth();
          width       -= 50;

          ImGui::SameLine(width);
          if (ImGui::Button("x"))
          {
            bool canDelete = true;
            if (wnd->m_name == g_3dViewport)
            {
              canDelete = false;
            }
            else if (wnd->m_name == g_2dViewport)
            {
              canDelete = false;
            }
            else if (wnd->m_name == g_IsoViewport)
            {
              canDelete = false;
            }

            if (canDelete)
            {
              g_app->m_windows.erase(g_app->m_windows.begin() + i);
            }
          }
          ImGui::EndGroup();
          ImGui::PopID();
        }
      };

      if (ImGui::BeginMenu("Viewport"))
      {
        handleMultiWindowFn(EditorViewport::StaticClass());
        handleMultiWindowFn(EditorViewport2d::StaticClass());

        if (ImGui::MenuItem("Add Viewport"))
        {
          EditorViewportPtr vp = MakeNewPtr<EditorViewport>();
          vp->Init({640.0f, 480.0f});
          g_app->m_windows.push_back(vp);
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Asset Browser"))
      {
        handleMultiWindowFn(FolderWindow::StaticClass());

        if (ImGui::MenuItem("Add Browser"))
        {
          FolderWindowPtr wnd = MakeNewPtr<FolderWindow>();
          wnd->m_name         = g_assetBrowserStr + "##" + std::to_string(wnd->m_id);
          wnd->IterateFolders(true);
          g_app->m_windows.push_back(wnd);
        }

        ImGui::EndMenu();
      }

      ImGui::Separator();

      ShowPersistentWindow<ConsoleWindow>(g_consoleStr);
      ShowPersistentWindow<OutlinerWindow>(g_outlinerStr);
      ShowPersistentWindow<PropInspectorWindow>(g_propInspector);
      ShowPersistentWindow<SimulationWindow>(g_simulationStr);
      ShowPersistentWindow<RenderSettingsWindow>(g_renderSettings);
      ShowPersistentWindow<StatsWindow>(g_statsView);

      ImGui::Separator();

      if (ImGui::MenuItem("Reset Layout"))
      {
        m_postponedActions.push_back([]() -> void { g_app->ResetUI(false); });
      }

#ifdef TK_DEBUG
      ImGui::Separator();

      if (!m_windowMenushowMetrics)
      {
        if (ImGui::MenuItem("Show Metrics"))
        {
          m_windowMenushowMetrics = true;
        }
      }

      if (!m_imguiSampleWindow)
      {
        if (ImGui::MenuItem("Imgui Sample"))
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
        // TODO Cihan Volitile window desstroys itself ( check the constructor )
        // yesnowindow causes a leak.
        StringInputWindow* inputWnd = new StringInputWindow("NewProject", true);
        inputWnd->m_inputVal        = "New Project";
        inputWnd->m_inputLabel      = "Name";
        inputWnd->m_hint            = "Project name";
        inputWnd->m_taskFn          = [](const String& val) { g_app->OnNewProject(val); };
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

      auto callPublisherForPlatformFn = [&](PublishPlatform publishPlatform, PublishConfig publishType)
      {
        if (publishPlatform == PublishPlatform::Android)
        {
          m_androidBuildWindow->OpenBuildWindow(publishType);
        }
        else
        {
          g_app->m_publishManager->Publish(publishPlatform, publishType);
        }
      };

      auto choosePublishPlatformFn = [&](PublishPlatform publishPlatform, bool developBuild = true)
      {
        if (ImGui::MenuItem("Debug"))
        {
          callPublisherForPlatformFn(publishPlatform, PublishConfig::Debug);
        }

        AddTooltipToLastItem("Builds the project in Debug config with debug info.\nDoes not pack the resources if "
                             "there is a pack already.");

        if (developBuild)
        {
          if (ImGui::MenuItem("Develop"))
          {
            callPublisherForPlatformFn(publishPlatform, PublishConfig::Develop);
          }

          AddTooltipToLastItem("Builds the project in Release config with debug info.\nDoes not pack the resources if "
                               "there is a pack already.");
        }

        if (ImGui::MenuItem("Deploy"))
        {
          callPublisherForPlatformFn(publishPlatform, PublishConfig::Deploy);
        }

        AddTooltipToLastItem("Builds the project in Release config without debug info.\nPacks the resources.");

        ImGui::EndMenu();
      };

      if (ImGui::BeginMenu("Publish"))
      {
        if (ImGui::MenuItem("Pack"))
        {
          g_app->PackResources();
        }

        if (ImGui::BeginMenu("Web"))
        {
          choosePublishPlatformFn(PublishPlatform::Web);
        }

        if (ImGui::BeginMenu("Android"))
        {
          choosePublishPlatformFn(PublishPlatform::Android, false);
        }

        if (ImGui::BeginMenu("Windows"))
        {
          choosePublishPlatformFn(PublishPlatform::Windows);
        }

        ImGui::EndMenu();
      }
    }

    void UI::ShowImportWindow()
    {
      static String load;
      if (!ImportData.ShowImportWindow || ImportData.Files.empty())
      {
        load.clear();
        return;
      }

      if (load.empty())
      {
        std::fstream importList;
        load = ConcatPaths({"..", "Utils", "Import", "importList.txt"});
        importList.open(load, std::ios::out);
        if (importList.is_open())
        {
          for (String& file : ImportData.Files)
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
        g_app->Import(load, ImportData.SubDir, ImportData.Overwrite);
        load.clear();
        ImportData.Files.clear();
        ImportData.ShowImportWindow = false;
        return;
      }

      ImGui::OpenPopup("Import");
      if (ImGui::BeginPopupModal("Import", NULL, ImGuiWindowFlags_AlwaysAutoResize))
      {
        String text;
        ImGui::Text("Import File:");
        for (size_t i = 0; i < ImportData.Files.size(); i++)
        {
          text = GetFileName(ImportData.Files[i]);
          ImGui::Text(" %s\n\n", text.c_str());
        }

        text = GetRelativeResourcePath(ImportData.ActiveView->GetPath());
        ImGui::Text("Import Target: %s\n\n", text.c_str());
        ImGui::Separator();

        static StringArray fails;
        if (!ImportData.Files.empty() && fails.empty())
        {
          for (int i = static_cast<int>(ImportData.Files.size()) - 1; i >= 0; i--)
          {
            bool canImp = g_app->CanImport(ImportData.Files[i]);
            if (!canImp)
            {
              fails.push_back(ImportData.Files[i]);
              ImportData.Files.erase(ImportData.Files.begin() + i);
            }
          }
        }

        if (!fails.empty())
        {
          ImGui::Text("Following imports failed due to:\nFile format is not "
                      "supported\nSupported formats: fbx, glb, obj, png, hdri & jpg");
          for (String& file : fails)
          {
            ImGui::Text("%s", file.c_str());
          }
          ImGui::Separator();
        }

        String importFolder;
        if (!ImportData.ActiveView->m_currRoot)
        {
          importFolder = ImportData.ActiveView->GetPath();
          importFolder = GetRelativeResourcePath(importFolder);
          if (ImportData.SubDir.length())
          {
            importFolder += GetPathSeparatorAsStr();
          }
        }
        importFolder += ImportData.SubDir;

        ImportData.ActiveView->Refresh();
        for (int i = static_cast<int>(ImportData.Files.size()) - 1; i >= 0; --i)
        {
          const String& file = ImportData.Files[i];
          String ext;
          DecomposePath(file, nullptr, nullptr, &ext);

          if (SupportedImageFormat(ext))
          {
            String viewPath    = ImportData.ActiveView->GetPath();
            String texturePath = TexturePath("");
            // Remove last string (PathSeperator)
            texturePath.pop_back();
            if (viewPath.find(texturePath) != String::npos)
            {
              // All images are copied to the active subfolder of Textures
              const String& dst = ImportData.ActiveView->GetPath();
              std::filesystem::copy(file, dst, std::filesystem::copy_options::overwrite_existing);
            }
            else
            {
              g_app->m_statusMsg = "Drop discarded.";
              TK_ERR("File isn't imported because it's not dropped into Textures folder.");
            }
            ImportData.Files.erase(ImportData.Files.begin() + i);
          }
        }
        if (ImportData.Files.size() == 0)
        {
          ImGui::EndPopup();
          ImportData.ShowImportWindow = false;
          ImGui::CloseCurrentPopup();
          return;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::Checkbox("Override", &ImportData.Overwrite);
        ImGui::PushItemWidth(100);
        ImGui::InputFloat("Scale", &ImportData.Scale);
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();

        ImGui::InputTextWithHint("Subdir", "optional", &ImportData.SubDir);
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
          if (g_app->Import(load, importFolder, ImportData.Overwrite) == -1)
          {
            // Fall back to search.
            ImGui::EndPopup();
            ImportData.ShowImportWindow = false;
            return;
          }

          load.clear();
          fails.clear();
          ImportData.Files.clear();
          ImportData.ShowImportWindow = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
          load.clear();
          fails.clear();
          ImportData.Files.clear();
          ImportData.ShowImportWindow = false;
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
          ImGui::Text("%s", s->c_str());
        }

        int itemCnt        = static_cast<int>(SearchFileData.searchPaths.size());
        const char** items = new const char*[itemCnt];
        for (int i = 0; i < itemCnt; i++)
        {
          items[i]       = SearchFileData.searchPaths[i].c_str();
          float textSize = ImGui::CalcTextSize(items[i]).x;
          maxSize        = glm::max(textSize, maxSize);
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
          String target = ImportData.SubDir;
          if (!ImportData.ActiveView->m_root)
          {
            target = ConcatPaths({ImportData.ActiveView->m_folder, ImportData.SubDir});
          }

          g_app->Import("", target, ImportData.Overwrite);
          ImportData.Files.clear();
          ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Abort", ImVec2(120, 0)))
        {
          ImportData.Files.clear();
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
      if (ImGui::BeginPopupModal(g_newSceneStr.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
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

    void UI::ShowBlocker()
    {
      if (!BlockerData.Show)
      {
        return;
      }

      IVec2 wp;
      SDL_GetWindowPosition(g_window, &wp.x, &wp.y);

      ImGuiIO& io = ImGui::GetIO();
      ImGui::SetNextWindowPos(ImVec2(wp.x + io.DisplaySize.x * 0.5f, wp.y + io.DisplaySize.y * 0.5f),
                              ImGuiCond_Always,
                              ImVec2(0.5f, 0.5f));

      ImGui::OpenPopup("Blocker");
      if (ImGui::BeginPopupModal("Blocker",
                                 NULL,
                                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
                                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
      {
        CenteredText(BlockerData.Message);

        if (BlockerData.ShowStatusMessages)
        {
          CenteredText(g_app->m_statusMsg);
        }

        if (BlockerData.ShowWaitingDots)
        {
          static int dotCnt   = 0;
          static float lastEp = GetElapsedMilliSeconds();
          float elp           = GetElapsedMilliSeconds();

          String dots[4]      = {"   .   ", "   ..   ", "   ...   ", " "};
          CenteredText(dots[dotCnt]);

          if (elp - lastEp > 500.0f)
          {
            lastEp = elp;
            dotCnt++;

            if (dotCnt > 3)
            {
              dotCnt = 0;
            }
          }
        }

        ImGui::EndPopup();
      }
    }

    bool UI::ButtonDecorless(StringView text, const Vec2& size, bool flipImage)
    {
      ImGui::PushStyleColor(ImGuiCol_Button, Vec4());
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Vec4());
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, Vec4());
      ImVec2 texCoords = flipImage ? ImVec2(1.0f, -1.0f) : ImVec2(1.0f, 1.0f);
      bool res         = ImGui::Button(text.data(), size);
      ImGui::PopStyleColor(3);
      return res;
    }

    bool UI::ImageButtonDecorless(uint textureID, const Vec2& size, bool flipImage)
    {
      ImGui::PushStyleColor(ImGuiCol_Button, Vec4());
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Vec4());
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, Vec4());
      ImVec2 texCoords = flipImage ? ImVec2(1.0f, -1.0f) : ImVec2(1.0f, 1.0f);
      bool res = ImGui::ImageButton(reinterpret_cast<void*>((intptr_t) textureID), size, ImVec2(0.0f, 0.0f), texCoords);
      ImGui::PopStyleColor(3);

      return res;
    }

    bool UI::ToggleButton(uint textureID, const Vec2& size, bool pushState)
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
      if (ImGui::ImageButton(reinterpret_cast<void*>((intptr_t) textureID), size))
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

    bool UI::ToggleButton(const String& text, const Vec2& size, bool pushState)
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

    float g_centeredTextOffset = 0.0f;

    bool UI::BeginCenteredTextButton(const String& text, const String& id)
    {
      Vec2 min  = ImGui::GetWindowContentRegionMin();
      Vec2 max  = ImGui::GetWindowContentRegionMax();
      Vec2 size = max - min;

      ImGui::AlignTextToFramePadding();
      ImVec2 tSize         = ImGui::CalcTextSize(text.c_str());
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

    void UI::CenteredText(const String& text)
    {
      float windowWidth = ImGui::GetWindowSize().x;
      float textWidth   = ImGui::CalcTextSize(text.c_str()).x;

      ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
      ImGui::Text(text.c_str());
    }

    String UI::EntityTypeToIcon(ClassMeta* Class)
    {
      String icon                                                   = ICON_FA_CUBE ICON_SPACE;

      static std::unordered_map<String, String> EntityTypeToIconMap = {
          {Camera::StaticClass()->Name,           ICON_FA_VIDEO_CAMERA ICON_SPACE},
          {Audio::StaticClass()->Name,            ICON_FA_FILE_AUDIO ICON_SPACE  },
          {EntityNode::StaticClass()->Name,       ICON_FA_ARROWS ICON_SPACE      },
          {Prefab::StaticClass()->Name,           ICON_FA_CUBES ICON_SPACE       },
          {Sphere::StaticClass()->Name,           ICON_FA_CIRCLE ICON_SPACE      },

          {Light::StaticClass()->Name,            ICON_FA_LIGHTBULB ICON_SPACE   },
          {PointLight::StaticClass()->Name,       ICON_FA_LIGHTBULB ICON_SPACE   },
          {SpotLight::StaticClass()->Name,        ICON_FA_LIGHTBULB ICON_SPACE   },
          {DirectionalLight::StaticClass()->Name, ICON_FA_SUN ICON_SPACE         },

          {Sky::StaticClass()->Name,              ICON_FA_SKYATLAS ICON_SPACE    },
          {GradientSky::StaticClass()->Name,      ICON_FA_SKYATLAS ICON_SPACE    },
      };

      auto entityIcon = EntityTypeToIconMap.find(Class->Name);
      if (entityIcon != EntityTypeToIconMap.end())
      {
        icon = entityIcon->second;
      }
      return icon;
    }

    void UI::ShowEntityTreeNodeContent(EntityPtr ntt)
    {
      String icon = UI::EntityTypeToIcon(ntt->Class());

      ImGui::SameLine();
      ImGui::Text((icon + ntt->GetNameVal()).c_str());

      // draw eye button for visibility
      ImGui::SameLine(ImGui::GetWindowWidth() - 65.0f);
      {
        icon          = ntt->GetVisibleVal() ? ICON_FA_EYE : ICON_FA_EYE_SLASH;
        float cursorY = ImGui::GetCursorPosY();
        ImGui::SetCursorPosY(cursorY - 2.5f);
        ImGui::PushID(static_cast<int>(ntt->GetIdVal()));
        if (UI::ButtonDecorless(icon, ImVec2(18.0f, 15.0f), false))
        {
          ntt->SetVisibility(!ntt->GetVisibleVal(), true);
        }
        ImGui::PopID();
      }

      // draw lock button for locking and unlocking the entity
      ImGui::SameLine(ImGui::GetWindowWidth() - 40.0f);
      {
        icon          = ntt->GetTransformLockVal() ? ICON_FA_LOCK : ICON_FA_UNLOCK;
        float cursorY = ImGui::GetCursorPosY();
        ImGui::SetCursorPosY(cursorY - 2.5f);
        ImGui::PushID(static_cast<int>(ntt->GetIdVal()));
        if (UI::ButtonDecorless(icon, ImVec2(18.0f, 15.0f), false))
        {
          ntt->SetTransformLock(!ntt->GetTransformLockVal(), true);
        }
        ImGui::PopID();
      }
    }

    bool UI::IsKeyboardCaptured() { return ImGui::GetIO().WantCaptureKeyboard; }

    void UI::AddTooltipToLastItem(const char* tip)
    {
      if (ImGui::IsItemHovered())
      {
        ImGui::SetItemTooltip(tip);
      }
    }

  } // namespace Editor
} // namespace ToolKit
