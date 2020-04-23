#pragma once

#include "ToolKit.h"
#include "UI.h"
#include <functional>

namespace ToolKit
{
	namespace Editor
	{
		typedef std::pair<String, std::vector<String>> TagArg;
		typedef std::vector<TagArg> TagArgArray;
		TagArgArray::const_iterator GetTag(String tag, const TagArgArray& tagArgs);

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

		class ConsoleWindow : public Window
		{
		public:
			ConsoleWindow();
			virtual ~ConsoleWindow();
			virtual void Show() override;
			virtual Type GetType() override;

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
			std::vector<String> m_items;
			std::vector<String> m_commands;
			std::unordered_map<String, std::function<void(TagArgArray&)>> m_commandExecutors;

			std::vector < String> m_history;
			int m_historyPos; // -1: new line, 0..History.Size-1 browsing history.

			// ImGui Helpers.
			ImGuiTextFilter m_filter;
		};
	}
}