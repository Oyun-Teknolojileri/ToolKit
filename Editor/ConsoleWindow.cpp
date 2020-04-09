#include "stdafx.h"

#include "ConsoleWindow.h"
#include "GlobalDef.h"
#include "Primative.h"
#include "Mod.h"
#include "DebugNew.h"

// Executors
void ToolKit::Editor::ShowPickDebugExec(std::string args)
{
	if (args == "1")
	{
		g_app->m_showPickingDebug = true;
	}

	if (args == "0")
	{
		g_app->m_showPickingDebug = false;

		g_app->m_scene.RemoveEntity(StatePickingBase::m_dbgArrow->m_id);
		StatePickingBase::m_dbgArrow = nullptr;

		g_app->m_scene.RemoveEntity(StatePickingBase::m_dbgFrustum->m_id);
		StatePickingBase::m_dbgFrustum = nullptr;
	}
}

void ToolKit::Editor::ShowOverlayExec(std::string args)
{
	if (args == "1")
	{
		g_app->m_showOverlayUI = true;
	}

	if (args == "0")
	{
		g_app->m_showOverlayUI = false;
	}
}

extern void ToolKit::Editor::ShowModTransitionsExec(std::string args)
{
	if (args == "1")
	{
		g_app->m_showStateTransitionsDebug = true;
	}

	if (args == "0")
	{
		g_app->m_showStateTransitionsDebug = false;
	}
}

// ImGui ripoff. Portable helpers.
static int   Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
static char* Strdup(const char* str) { size_t len = strlen(str) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)str, len); }
static void  Strtrim(char* str) { char* str_end = str + strlen(str); while (str_end > str&& str_end[-1] == ' ') str_end--; *str_end = 0; }

ToolKit::Editor::ConsoleWindow::ConsoleWindow()
{
	CreateCommand(g_showPickDebugCmd, ShowPickDebugExec);
	CreateCommand(g_showOverlayUICmd, ShowOverlayExec);
	CreateCommand(g_showModTransitions, ShowModTransitionsExec);
}

ToolKit::Editor::ConsoleWindow::~ConsoleWindow()
{
}

void ToolKit::Editor::ConsoleWindow::Show()
{
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Once);
	ImGui::Begin(g_consoleStr.c_str(), &m_visible);
	{
		HandleStates();

		// Search bar.
		ImGui::Text("Search: ");
		ImGui::SameLine();

		m_filter.Draw("##Filter", 180);
		ImGui::Separator();

		// Output window.
		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
		for (size_t i = 0; i < m_items.size(); i++)
		{
			const char* item = m_items[i].c_str();
			if (!m_filter.PassFilter(item))
			{
				continue;
			}

			bool pop_color = false;
			if (strstr(item, g_errorStr.c_str())) 
			{ 
				ImGui::PushStyleColor(ImGuiCol_Text, GLM4IMVEC(g_consoleErrorColor));
				pop_color = true; 
			}
			else if (strstr(item, g_commandStr.c_str()))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, GLM4IMVEC(g_consoleCommandColor));
				pop_color = true;
			}
			else if (strstr(item, g_warningStr.c_str()))
			{ 
				ImGui::PushStyleColor(ImGuiCol_Text, GLM4IMVEC(g_consoleWarningColor));
				pop_color = true;
			}
			else // Than its a memo.
			{
				ImGui::PushStyleColor(ImGuiCol_Text, GLM4IMVEC(g_consoleMemoColor));
				pop_color = true;
			}

			ImGui::TextUnformatted(item);
			if (pop_color)
			{
				ImGui::PopStyleColor();
			}
		}

		if (m_scrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		{
			ImGui::SetScrollHereY(1.0f);
			m_scrollToBottom = false;
		}

		ImGui::PopStyleVar();
		ImGui::EndChild();

		ImGui::Text("Command: ");
		ImGui::SameLine();

		// Command window.
		bool reclaim_focus = false;
		static char inputBuff[256];
		if (ImGui::InputText(
			"##Input",
			inputBuff,
			IM_ARRAYSIZE(inputBuff),
			ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory,
			[](ImGuiInputTextCallbackData* data)->int { return ((ConsoleWindow*)(data->UserData))->TextEditCallback(data); },
			(void*)this))
		{
			char* s = inputBuff;
			Strtrim(s);
			if (s[0])
			{
				ExecCommand(s);
			}
			strcpy_s(s, sizeof(inputBuff), "");
			reclaim_focus = true;
		}

		// Auto-focus on window apparition
		ImGui::SetItemDefaultFocus();
		if (reclaim_focus)
		{
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		}
	}
	ImGui::End();
}

ToolKit::Editor::Window::Type ToolKit::Editor::ConsoleWindow::GetType()
{
	return Type::Console;
}

void ToolKit::Editor::ConsoleWindow::AddLog(const std::string& log, LogType type)
{
	std::string prefixed;
	switch (type)
	{
	case LogType::Error:
		prefixed = g_errorStr + log;
		break;
	case LogType::Warning:
		prefixed = g_warningStr + log;
		break;
	case LogType::Command:
		prefixed = g_commandStr + log;
		break;
	case LogType::Memo:
	default:
		prefixed = g_memoStr + log;
		break;
	}

	m_items.push_back(prefixed);
	m_scrollToBottom = true;
}

void ToolKit::Editor::ConsoleWindow::AddLog(const std::string& log, const std::string& tag)
{
	std::string prefixed = "[" + tag + "] " + log;
	m_items.push_back(prefixed);
	m_scrollToBottom = true;
}

void ToolKit::Editor::ConsoleWindow::ClearLog()
{
	m_items.clear();
}

void ToolKit::Editor::ConsoleWindow::ExecCommand(const std::string& commandLine)
{
	// Separate command and args.
	size_t argsIndx = commandLine.find_first_of(" ");
	std::string args, cmd;
	if (argsIndx != std::string::npos)
	{
		args = commandLine.substr(argsIndx + 1);
		cmd = commandLine.substr(0, argsIndx);
	}
	else
	{
		cmd = commandLine;
	}

	// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
	m_historyPos = -1;
	for (int i = (int)m_history.size() - 1; i >= 0; i--)
	{
		if (Stricmp(m_history[i].c_str(), commandLine.c_str()) == 0)
		{
			m_history.erase(m_history.begin() + i);
			break;
		}
	}
	m_history.push_back(commandLine);

	// Process command.
	char buffer[256];
	if (m_commandExecutors.find(cmd) != m_commandExecutors.end())
	{
		AddLog(commandLine, LogType::Command);
		m_commandExecutors[cmd](args);
	}
	else
	{
		sprintf_s(buffer, sizeof(buffer), "Unknown command: '%s'\n", cmd.c_str());
		AddLog(buffer, LogType::Error);
	}

	m_scrollToBottom = true;
}

// Mostly ripoff from the ImGui Console Example.
int ToolKit::Editor::ConsoleWindow::TextEditCallback(ImGuiInputTextCallbackData* data)
{
	char buffer[256];

	switch (data->EventFlag)
	{
		case ImGuiInputTextFlags_CallbackCompletion:
		{
			// Locate beginning of current word
			const char* word_end = data->Buf + data->CursorPos;
			const char* word_start = word_end;
			while (word_start > data->Buf)
			{
				const char c = word_start[-1];
				if (c == ' ' || c == '\t' || c == ',' || c == ';')
				{
					break;
				}
				word_start--;
			}

			// Build a list of candidates
			std::vector<std::string> candidates;
			for (size_t i = 0; i < m_commands.size(); i++)
			{
				if (Strnicmp(m_commands[i].c_str(), word_start, (int)(word_end - word_start)) == 0)
				{
					candidates.push_back(m_commands[i]);
				}
			}

			if (candidates.empty())
			{
				// No match
				sprintf_s(buffer, sizeof(buffer), "No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
				AddLog(buffer);
			}
			else if (candidates.size() == 1)
			{
				// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0].c_str());
				data->InsertChars(data->CursorPos, " ");
			}
			else
			{
				// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
				int match_len = (int)(word_end - word_start);
				for (;;)
				{
					int c = 0;
					bool all_candidates_matches = true;
					for (size_t i = 0; i < candidates.size() && all_candidates_matches; i++)
						if (i == 0)
						{
							c = toupper(candidates[i][match_len]);
						}
						else if (c == 0 || c != toupper(candidates[i][match_len]))
						{
							all_candidates_matches = false;
						}
					if (!all_candidates_matches)
					{
						break;
					}
					match_len++;
				}

				if (match_len > 0)
				{
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0].c_str(), candidates[0].c_str() + match_len);
				}

				// List matches
				AddLog("Possible matches:\n");
				for (size_t i = 0; i < candidates.size(); i++)
				{
					sprintf_s(buffer, sizeof(buffer), "- %s\n", candidates[i].c_str());
					AddLog(buffer);
				}
			}

			break;
		}
		case ImGuiInputTextFlags_CallbackHistory:
		{
			const int prev_history_pos = m_historyPos;
			if (data->EventKey == ImGuiKey_UpArrow)
			{
				if (m_historyPos == -1)
				{
					m_historyPos = (int)m_history.size() - 1;
				}
				else if (m_historyPos > 0)
				{
					m_historyPos--;
				}
			}
			else if (data->EventKey == ImGuiKey_DownArrow)
			{
				if (m_historyPos != -1)
				{
					if (++m_historyPos >= m_history.size())
					{
						m_historyPos = -1;
					}
				}
			}

			// A better implementation would preserve the data on the current input line along with cursor position.
			if (prev_history_pos != m_historyPos)
			{
				const char* history_str = (m_historyPos >= 0) ? m_history[m_historyPos].c_str() : "";
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, history_str);
			}
		}
	}

	return 0;
}

void ToolKit::Editor::ConsoleWindow::CreateCommand(const std::string& command, std::function<void(std::string)> executor)
{
	m_commands.push_back(command);
	m_commandExecutors[command] = executor;
}
