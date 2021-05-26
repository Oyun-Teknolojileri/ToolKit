#pragma once

#include "SDL.h"
#include "App.h"
#include "Types.h"

namespace ToolKit
{
  namespace Editor
  {

    // Handles.
    extern SDL_Window* g_window;
    extern SDL_GLContext g_context;
    extern Editor::App* g_app;
    extern bool g_running;

    // UI Strings
    const String g_consoleStr("Console");
    const String g_viewportStr("Viewport");
    const String g_assetBrowserStr("AssetBrowser");
    const String g_outlinerStr("Outliner");
    const String g_memoStr("");
    const String g_errorStr("[Err] ");
    const String g_warningStr("[Wrn] ");
    const String g_commandStr("# ");

    // Colors and materials.
    const String g_gridMaterialName("TK_EDITOR_GRID");
    const Vec3 g_gridAxisBlue = Vec3(0.196f, 0.541f, 0.905f);
    const Vec3 g_gridAxisRed = Vec3(0.89f, 0.239f, 0.341f);
    const Vec3 g_gizmoRed = Vec3(0.89f, 0.239f, 0.341f);
    const Vec3 g_gizmoGreen = Vec3(0.537f, 0.831f, 0.07f);
    const Vec3 g_gizmoBlue = Vec3(0.196f, 0.541f, 0.905f);
    const Vec3 g_gizmoLocked = Vec3(0.3f);
    const std::vector<Vec3> g_gizmoColor = { g_gizmoRed, g_gizmoGreen, g_gizmoBlue };
    const Vec4 g_selectBoxWindowColor = Vec4(0.4f, 0.4f, 0.4f, 0.4f);
    const Vec4 g_selectBoxBorderColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    const Vec3 g_selectHighLightPrimaryColor = Vec3(1.0f, 0.627f, 0.156f);
    const Vec3 g_selectHighLightSecondaryColor = Vec3(0.898f, 0.352f, 0.031f);
    const Vec4 g_consoleErrorColor = Vec4(1.0f, 0.4f, 0.4f, 1.0f);
    const Vec4 g_consoleCommandColor = Vec4(1.0f, 0.8f, 0.6f, 1.0f);
    const Vec4 g_consoleWarningColor = Vec4(0.3f, 0.8f, 0.3f, 1.0f);
    const Vec4 g_consoleMemoColor = Vec4(0.7f, 0.7f, 0.7f, 1.0f);

    // Editor settings.
    const size_t g_maxUndoCount = 50;

    // Utility functions.		
    inline float MilisecToSec(float ms)
    {
      return ms / 1000.0f;
    }

    #define GLM2IMVEC(v) *reinterpret_cast<const ImVec2*>(&v)
    #define GLM4IMVEC(v) *reinterpret_cast<const ImVec4*>(&v)
    #define IMVEC2GLM(v) *reinterpret_cast<const glm::vec2*>(&v)
    #define Convert2ImGuiTexture(TexturePtr) (void*)(intptr_t)(TexturePtr->m_textureId)
  }
}