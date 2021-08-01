#pragma once

#include "ToolKit.h"
#include "UI.h"
#include <functional>

namespace ToolKit
{
  namespace Editor
  {
    typedef std::pair<String, StringArray> TagArg;
    typedef std::vector<TagArg> TagArgArray;
    typedef TagArgArray::const_iterator TagArgCIt;
    TagArgArray::const_iterator GetTag(String tag, const TagArgArray& tagArgs);
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

    const String g_importSlientCmd("SetImportSlient");
    void SetImportSlient(TagArgArray tagArgs);

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

    // Command errors
    const String g_noValidEntity("No valid entity");

    class ConsoleWindow : public Window
    {
    public:
      ConsoleWindow(XmlNode* node);
      ConsoleWindow();
      virtual ~ConsoleWindow();
      virtual void Show() override;
      virtual Type GetType() const override;

      // Functions.
      enum class LogType
      {
        Memo,
        Error,
        Warning,
        Command
      };
      void AddLog(const String& log, LogType type = LogType::Memo);
      void AddLog(const String& log, const String& tag);
      void ClearLog();
      void ExecCommand(const String& commandLine);
      void ParseCommandLine(String commandLine, String& command, TagArgArray& tagArgs);

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

      std::vector <String> m_history;
      int m_historyPos = -1; // -1: new line, 0..History.Size-1 browsing history.

      // ImGui Helpers.
      ImGuiTextFilter m_filter;
    };
  }
}