#pragma once

#include "ToolKit.h"
#include "UI.h"
#include <functional>

namespace ToolKit
{
	namespace Editor
	{
		// Commands & Executors.
		const String g_showPickDebugCmd("ShowPickGeometry");
		extern void ShowPickDebugExec(String args);

		const String g_showOverlayUICmd("ShowOverlayUI");
		extern void ShowOverlayExec(String args);

		const String g_showOverlayUIAlwaysCmd("ShowOverlayUIAlways");
		extern void ShowOverlayAlwaysExec(String args);

		const String g_showModTransitionsCmd("ShowModTransitions");
		extern void ShowModTransitionsExec(String args);

		const String g_SetTransformCmd("SetTransform");
		extern void SetTransformExec(String args);

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

		private:
			// Command line word processing. Auto-complete and history lookups.
			int TextEditCallback(ImGuiInputTextCallbackData* data);
			void CreateCommand(const String& command, std::function<void(String)> executor);
			
		private:
			// States.
			bool m_scrollToBottom = false;

			// Buffers.
			std::vector<String> m_items;
			std::vector<String> m_commands;
			std::unordered_map<String, std::function<void(String)>> m_commandExecutors;

			std::vector < String> m_history;
			int m_historyPos; // -1: new line, 0..History.Size-1 browsing history.

			// ImGui Helpers.
			ImGuiTextFilter m_filter;
		};
	}
}