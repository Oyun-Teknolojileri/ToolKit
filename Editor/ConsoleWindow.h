/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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

    // Command errors
    const String g_noValidEntity("No valid entity");

    class ConsoleWindow : public Window
    {
     public:
      ConsoleWindow();
      virtual ~ConsoleWindow();
      void Show() override;
      Type GetType() const override;

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
      TKMap<String, std::function<void(TagArgArray&)>> m_commandExecutors;

      // Command text
      String m_command    = "";
      String m_filter     = "";
      bool m_reclaimFocus = false;

      StringArray m_history;
      // -1: new line, 0..History.Size-1 browsing history.
      int m_historyPos = -1;
    };

  } // namespace Editor
} // namespace ToolKit
