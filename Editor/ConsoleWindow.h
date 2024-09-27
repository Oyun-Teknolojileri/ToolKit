/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "UI.h"

namespace ToolKit
{
  namespace Editor
  {

    typedef std::pair<String, StringArray> TagArg;
    typedef std::vector<TagArg> TagArgArray;
    typedef TagArgArray::const_iterator TagArgCIt;
    TagArgCIt GetTag(const String& tag, const TagArgArray& tagArgs);
    void ParseVec(Vec3& vec, TagArgCIt tagIt);

    // Commands & Executors.
    const String g_showPickDebugCmd("ShowPickGeometry");
    void ShowPickDebugExec(TagArgArray tagArgs);

    const String g_showOverlayUICmd("ShowOverlayUI");
    void ShowOverlayExec(TagArgArray tagArgs);

    const String g_showOverlayUIAlwaysCmd("ShowOverlayUIAlways");
    void ShowOverlayAlwaysExec(TagArgArray tagArgs);

    const String g_showModTransitionsCmd("ShowModTransitions");
    void ShowModTransitionsExec(TagArgArray tagArgs);

    const String g_setTransformCmd("SetTransform");
    void SetTransformExec(TagArgArray tagArgs);

    const String g_transformCmd("Transform");
    void TransformExec(TagArgArray tagArgs);

    const String g_setCameraTransformCmd("SetCameraTransform");
    void SetCameraTransformExec(TagArgArray tagArgs);

    const String g_resetCameraCmd("ResetCamera");
    void ResetCameraExec(TagArgArray tagArgs);

    const String g_getTransformCmd("GetTransform");
    void GetTransformExec(TagArgArray tagArgs);

    const String g_setTransformOrientationCmd("SetTransformOrientation");
    void SetTransformOrientationExec(TagArgArray tagArgs);

    const String g_importSlientCmd("ImportSlient");
    void ImportSlient(TagArgArray tagArgs);

    const String g_selectByTag("SelectByTag");
    void SelectByTag(TagArgArray tagArgs);

    const String g_lookAt("LookAt");
    void LookAt(TagArgArray tagArgs);

    const String g_applyTransformToMesh("ApplyTransformToMesh");
    void ApplyTransformToMesh(TagArgArray tagArgs);

    const String g_saveMesh("SaveMesh");
    void SaveMesh(TagArgArray tagArgs);

    const String g_showSelectionBoundary("ShowSelectionBoundary");
    void ShowSelectionBoundary(TagArgArray tagArgs);

    const String g_showGraphicsApiLogs("ShowGraphicsApiLogs");
    void ShowGraphicsApiLogs(TagArgArray tagArgs);

    const String g_setWorkspaceDir("SetWorkspaceDir");
    void SetWorkspaceDir(TagArgArray tagArgs);

    const String g_loadPlugin("LoadPlugin");
    void LoadPlugin(TagArgArray tagArgs);

    const String g_showShadowFrustum("ShowShadowFrustum");
    void ShowShadowFrustum(TagArgArray tagArgs);

    const String g_selectEffectingLights("SelectAllEffectingLights");
    void SelectAllEffectingLights(TagArgArray tagArgs);

    const String g_checkSceneHealth("CheckSceneHealth");
    void CheckSceneHealth(TagArgArray tagArgs);

    const String g_showSceneBoundary("ShowSceneBoundary");
    void ShowSceneBoundary(TagArgArray tagArgs);

    const String g_showBVHNodes("ShowBVHNodes");
    void ShowBVHNodes(TagArgArray tagArgs);

    const String g_deleteSelection("DeleteSelection");
    void DeleteSelection(TagArgArray tagArgs);

    // Command errors
    const String g_noValidEntity("No valid entity");

    class ConsoleWindow : public Window
    {
     public:
      TKDeclareClass(ConsoleWindow, Window);

      ConsoleWindow();
      virtual ~ConsoleWindow();
      void Show() override;

      void AddLog(const String& log, LogType type = LogType::Memo);
      void AddLog(const String& log, const String& tag);
      void ClearLog();
      void ExecCommand(const String& commandLine);
      void ParseCommandLine(const String& commandLine, String& command, TagArgArray& tagArgs);

     private:
      // Command line word processing. Auto-complete and history lookups.
      int TextEditCallback(ImGuiInputTextCallbackData* data);
      void CreateCommand(const String& command, std::function<void(TagArgArray)> executor);

     private:
      // States.
      bool m_scrollToBottom = false;

      // Buffers.
      StringArray m_items;
      StringArray m_commands;
      std::unordered_map<String, std::function<void(TagArgArray&)>> m_commandExecutors;

      // Command text
      String m_command    = "";
      String m_filter     = "";
      bool m_reclaimFocus = false;

      StringArray m_history;
      // -1: new line, 0..History.Size-1 browsing history.
      int m_historyPos = -1;
    };

    typedef std::shared_ptr<ConsoleWindow> ConsoleWindowPtr;

  } // namespace Editor
} // namespace ToolKit
