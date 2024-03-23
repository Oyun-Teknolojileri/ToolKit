/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "IconsFontAwesome.h"
#include "Window.h"

#include <ImGui/imgui.h>
#include <ImGui/misc/cpp/imgui_stdlib.h>
#include <Object.h>
#include <Types.h>

namespace ToolKit
{
  namespace Editor
  {

    // Global Style Decelerations
    extern const float g_indentSpacing;
    extern const int g_treeNodeFlags;

    enum class Theme
    {
      Light = 1,
      Dark  = 2,
      Grey  = 3
    };

    class UI
    {
     public:
      static void Init();
      static void UnInit();
      static void HeaderText(const char* text);
      static void PushBoldFont();
      static void PopBoldFont();
      static void ShowDock();
      static void InitIcons();
      static void InitTheme();
      static void InitSettings();
      static void ShowUI();
      static void BeginUI();
      static void EndUI();
      static void ShowAppMainMenuBar();
      static void ShowMenuFile();
      static void ShowMenuWindows();
      static void ShowMenuProjects();
      static void ShowImportWindow();
      static void ShowSearchForFilesWindow();
      static void HelpMarker(const String& key, const char* desc, float wait = m_hoverTimeForHelp);
      static void ShowNewSceneWindow();
      static void ShowBlocker();

      // Custom widgets.
      static bool ButtonDecorless(StringView text, const Vec2& size, bool flipImage);
      static bool ImageButtonDecorless(uint textureID, const Vec2& size, bool flipImage);
      static bool ToggleButton(uint textureID, const Vec2& size, bool pushState);
      static bool ToggleButton(const String& text, const Vec2& size, bool pushState);
      static bool BeginCenteredTextButton(const String& text, const String& id = "");
      static void EndCenteredTextButton();
      static void CenteredText(const String& text);

      /**
       * Returns a font / icon for Entity classes.
       * @Class is the Class type to query font for.
       * @return FontAwesome string (icon) from any given entity class.
       */
      static String EntityTypeToIcon(ClassMeta* Class); //!< Returns
      static void ShowEntityTreeNodeContent(EntityPtr ntt);

      /**
       * Can be used to see if ui is using the keyboard for input. Most likely
       * usage is to check if user typing text to an input field.
       */
      static bool IsKeyboardCaptured();

      static void AddTooltipToLastItem(const char* tip);

     public:
      static bool m_showNewSceneWindow;
      static bool m_imguiSampleWindow;
      static bool m_windowMenushowMetrics;
      static float m_hoverTimeForHelp;

      static struct Blocker
      {
        bool Show               = false;
        bool ShowStatusMessages = false;
        bool ShowWaitingDots    = false;
        String Message          = "Working";
      } BlockerData;

      static struct Import
      {
        bool ShowImportWindow = false;
        bool Overwrite        = false;
        StringArray Files;
        String SubDir;
        float Scale                  = 1.0f;
        class FolderView* ActiveView = nullptr;
      } ImportData;

      static struct SearchFile
      {
        bool showSearchFileWindow = false;
        StringArray missingFiles;
        StringArray searchPaths;
      } SearchFileData;

      // Some actions needed to be run after ui rendered.
      static std::vector<std::function<void()>> m_postponedActions;

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
      static TexturePtr m_worldIcon;
      static TexturePtr m_axisIcon;
      static TexturePtr m_playIcon;
      static TexturePtr m_pauseIcon;
      static TexturePtr m_stopIcon;
      static TexturePtr m_vsCodeIcon;
      static TexturePtr m_collectionIcon;
      static TexturePtr m_arrowsIcon;
      static TexturePtr m_lockIcon;
      static TexturePtr m_visibleIcon;
      static TexturePtr m_invisibleIcon;
      static TexturePtr m_lockedIcon;
      static TexturePtr m_unlockedIcon;
      static TexturePtr m_viewZoomIcon;
      static TexturePtr m_gridIcon;
      static TexturePtr m_skyIcon;
      static TexturePtr m_closeIcon;
      static TexturePtr m_phoneRotateIcon;
      static TexturePtr m_studioLightsToggleIcon;
      static TexturePtr m_anchorIcn;
      static TexturePtr m_prefabIcn;
      static TexturePtr m_buildIcn;
      static TexturePtr m_addIcon;
      static TexturePtr m_sphereIcon;
      static TexturePtr m_cubeIcon;
      static TexturePtr m_shaderBallIcon;
      static TexturePtr m_diskDriveIcon;
      static TexturePtr m_packageIcon;
      static TexturePtr m_objectDataIcon;
      static TexturePtr m_sceneIcon;

      struct AnchorPresetImages
      {
        static const uint presetCount          = 16;
        StringView m_presetNames[presetCount]  = {"Top Left",
                                                  "Top Middle",
                                                  "Top Right",
                                                  "Top Horizontal",
                                                  "Middle Left",
                                                  "Middle Middle",
                                                  "Middle Right",
                                                  "Middle Horizontal",
                                                  "Bottom Left",
                                                  "Bottom Middle",
                                                  "Bottom Right",
                                                  "Bottom Horizontal",
                                                  "Vertical Left",
                                                  "Vertical Middle",
                                                  "Vertical Right",
                                                  "Whole"};
        TexturePtr m_presetImages[presetCount] = {};
      };

      static AnchorPresetImages m_anchorPresetIcons;
    };
  } // namespace Editor
} // namespace ToolKit
