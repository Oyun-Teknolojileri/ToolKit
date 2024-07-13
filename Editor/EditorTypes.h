/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

extern struct SDL_Window* g_window;
extern void* g_context;

#ifdef TK_WIN
  #if defined(TK_EDITOR_DLL_EXPORT)
    #define TK_EDITOR_API __declspec(dllexport)
  #else
    #define TK_EDITOR_API __declspec(dllimport)
  #endif
#endif

namespace ToolKit
{
  namespace Editor
  {

    typedef std::shared_ptr<class AndroidBuildWindow> AndroidBuildWindowPtr;

    // Handles.
    extern class App* g_app;
    extern bool g_running;

    // UI Strings
    const String g_consoleStr("Console");
    const String g_assetBrowserStr("Asset Browser");
    const String g_outlinerStr("Outliner");
    const String g_propInspector("Property Inspector");
    const String g_renderSettings("Render Settings");
    const String g_statsView("Statistics");
    const String g_matInspector("Material Inspector");
    const String g_simulationWindowStr("Simulation");
    const String g_pluginWindow("Plugins");
    const String g_memoStr("Mem");
    const String g_errorStr("Err");
    const String g_warningStr("Wrn");
    const String g_successStr("Suc");
    const String g_commandStr("#");
    const String g_newSceneStr("New Scene");
    const String g_viewportStr("Viewport");
    const String g_2dViewport("2D Viewport");
    const String g_3dViewport("3D Viewport");
    const String g_IsoViewport("ISO Viewport");
    const String g_simulationViewStr("Simulation Viewport");
    const String g_workspaceFile("Workspace.settings");
    const String g_uiLayoutFile("UILayout.ini");
    const String g_editorSettingsFile("Editor.settings");
    const String g_statusNoTerminate("#nte");
    static const StringView XmlNodePath("path");

    // Colors and materials.
    // Reversed gamma correction for all colors
    const String g_gridMaterialName("TK_EDITOR_GRID");
    const Vec3 g_gridAxisBlue                  = Vec3(0.196f, 0.541f, 0.905f);
    const Vec3 g_gridAxisRed                   = Vec3(0.89f, 0.239f, 0.341f);
    const Vec3 g_gridAxisGreen                 = Vec3(0.537f, 0.831f, 0.07f);
    const Vec3 g_gizmoRed                      = Vec3(0.89f, 0.239f, 0.341f);
    const Vec3 g_gizmoGreen                    = Vec3(0.537f, 0.831f, 0.07f);
    const Vec3 g_gizmoBlue                     = Vec3(0.196f, 0.541f, 0.905f);
    const Vec3 g_cameraGizmoColor              = Vec3(0.0f);
    const Vec3 g_lightGizmoColor               = Vec3(0.0f);
    const Vec3 g_environmentGizmoColor         = Vec3(0.0f);
    const Vec3 g_gizmoLocked                   = Vec3(0.3f);
    const std::vector<Vec3> g_gizmoColor       = {g_gizmoRed, g_gizmoGreen, g_gizmoBlue};
    const Vec4 g_wndBgColor                    = Vec4(0.007024517f, 0.00959683f, 0.018735119f, 1.0f);

    const Vec4 g_selectBoxWindowColor          = Vec4(0.4f, 0.4f, 0.4f, 0.4f);
    const Vec4 g_selectBoxBorderColor          = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    const Vec4 g_selectHighLightPrimaryColor   = Vec4(1.0f, 0.627f, 0.156f, 1.0f);
    const Vec4 g_selectHighLightSecondaryColor = Vec4(0.898f, 0.352f, 0.031f, 1.0f);

    const Vec3 g_anchorColor                   = Vec3(1.f);
    const Vec4 g_anchorGuideLineColor          = Vec4(0.032276204f, 0.681419299f, 0.681419299f, 0.456263458f);

    const Vec4 g_consoleErrorColor             = Vec4(1.0f, 0.4f, 0.4f, 1.0f);
    const Vec4 g_consoleCommandColor           = Vec4(1.0f, 0.8f, 0.6f, 1.0f);
    const Vec4 g_consoleWarningColor           = Vec4(0.9f, 0.8f, 0.1f, 1.0f);
    const Vec4 g_consoleSuccessColor           = Vec4(0.4f, 0.94f, 0.4f, 1.0f);
    const Vec4 g_consoleMemoColor              = Vec4(0.7f, 0.7f, 0.7f, 1.0f);

    const Vec4 g_blueTintButtonColor           = Vec4(0.043f, 0.173f, 0.325f, 1.0f);
    const Vec4 g_blueTintButtonHoverColor      = Vec4(0.032f, 0.208f, 0.456f, 1.0f);
    const Vec4 g_blueTintButtonActiveColor     = Vec4(0.018f, 0.247f, 0.612f, 1.0f);

    const Vec4 g_greenTintButtonColor          = Vec4(0.0949f, 0.325f, 0.044f, 1.0f);
    const Vec4 g_greenTintButtonHoverColor     = Vec4(0.099f, 0.456f, 0.032f, 1.0f);
    const Vec4 g_greenTintButtonActiveColor    = Vec4(0.095f, 0.612f, 0.018f, 1.0f);

    const Vec4 g_redTintButtonColor            = Vec4(0.325f, 0.043f, 0.043f, 1.0f);
    const Vec4 g_redTintButtonHoverColor       = Vec4(0.456f, 0.032f, 0.032f, 1.0f);
    const Vec4 g_redTintButtonActiveColor      = Vec4(0.612f, 0.018f, 0.018f, 1.0f);

    // Editor settings.
    const size_t g_maxUndoCount                = 50;
    const UVec2 g_max2dGridSize(100 * 100 * 2);

#define Convert2ImGuiTexture(TexturePtr) (void*) (intptr_t) (TexturePtr->m_textureId) // NOLINT

#define ConvertUIntImGuiTexture(uint)    (void*) (intptr_t) (uint)

  } // namespace Editor
} // namespace ToolKit
