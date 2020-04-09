#pragma once

#include "ToolKit.h"
#include "UI.h"
#include <functional>

namespace ToolKit
{
	namespace Editor
	{
		// Commands & Executors.
		const std::string g_showPickDebugCmd("ShowPickGeometry");
		extern void ShowPickDebugExec(std::string args);

		const std::string g_showOverlayUICmd("ShowOverlayUI");
		extern void ShowOverlayExec(std::string args);

		const std::string g_showModTransitions("ShowModTransitions");
		extern void ShowModTransitionsExec(std::string args);

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
			void AddLog(const std::string& log, LogType type = LogType::Memo);
			void AddLog(const std::string& log, const std::string& tag);
			void ClearLog();
			void ExecCommand(const std::string& commandLine);

		private:
			// Command line word processing. Auto-complete and history lookups.
			int TextEditCallback(ImGuiInputTextCallbackData* data);
			void CreateCommand(const std::string& command, std::function<void(std::string)> executor);
			
		private:
			// States.
			bool m_scrollToBottom = false;

			// Buffers.
			std::vector<std::string> m_items;
			std::vector<std::string> m_commands;
			std::unordered_map<std::string, std::function<void(std::string)>> m_commandExecutors;

			std::vector < std::string> m_history;
			int m_historyPos; // -1: new line, 0..History.Size-1 browsing history.

			// ImGui Helpers.
			ImGuiTextFilter m_filter;
		};
	}
}