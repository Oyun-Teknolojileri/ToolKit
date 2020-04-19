#include "stdafx.h"

#include "ConsoleWindow.h"
#include "GlobalDef.h"
#include "Primative.h"
#include "Mod.h"
#include "Entity.h"
#include "Node.h"
#include "Directional.h"
#include "Viewport.h"
#include "DebugNew.h"

namespace ToolKit
{
	namespace Editor
	{

		TagArgArray::const_iterator GetTag(String tag, const TagArgArray& tagArgs)
		{
			for (TagArgArray::const_iterator ta = tagArgs.cbegin(); ta != tagArgs.cend(); ta++)
			{
				if (ta->first == tag)
				{
					return ta;
				}
			}

			return tagArgs.end();
		}

		// Executors
		void ShowPickDebugExec(TagArgArray tagArgs)
		{
			if (tagArgs.empty())
			{
				return;
			}
			if (tagArgs.front().second.empty())
			{
				return;
			}

			String args = tagArgs.front().second.front();

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

		void ShowOverlayExec(TagArgArray tagArgs)
		{
			if (tagArgs.empty())
			{
				return;
			}
			if (tagArgs.front().second.empty())
			{
				return;
			}

			String args = tagArgs.front().second.front();

			if (args == "1")
			{
				g_app->m_showOverlayUI = true;
			}

			if (args == "0")
			{
				g_app->m_showOverlayUI = false;
			}
		}

		void ShowOverlayAlwaysExec(TagArgArray tagArgs)
		{
			if (tagArgs.empty())
			{
				return;
			}
			if (tagArgs.front().second.empty())
			{
				return;
			}

			String args = tagArgs.front().second.front();

			if (args == "1")
			{
				g_app->m_showOverlayUIAlways = true;
			}

			if (args == "0")
			{
				g_app->m_showOverlayUIAlways = false;
			}
		}

		void ShowModTransitionsExec(TagArgArray tagArgs)
		{
			if (tagArgs.empty())
			{
				return;
			}
			if (tagArgs.front().second.empty())
			{
				return;
			}

			String args = tagArgs.front().second.front();

			if (args == "1")
			{
				g_app->m_showStateTransitionsDebug = true;
			}

			if (args == "0")
			{
				g_app->m_showStateTransitionsDebug = false;
			}
		}

		void TransformInternal(TagArgArray tagArgs, bool set)
		{
			Entity* e = g_app->m_scene.GetCurrentSelection();
			if (e == nullptr)
			{
				return;
			}

			TransformationSpace ts = TransformationSpace::TS_WORLD;
			TagArgArray::const_iterator transformSpaceTag = GetTag("ts", tagArgs);
			if (transformSpaceTag != tagArgs.end())
			{
				if (!transformSpaceTag->second.empty())
				{
					String tsStr = transformSpaceTag->second.front();
					if (tsStr == "world")
					{
						ts = TransformationSpace::TS_WORLD;
					}

					if (tsStr == "parent")
					{
						ts = TransformationSpace::TS_PARENT;
					}

					if (tsStr == "local")
					{
						ts = TransformationSpace::TS_LOCAL;
					}
				}
			}

			for (TagArg& tagIt : tagArgs)
			{
				String tag = tagIt.first;
				std::vector<String>& args = tagIt.second;

				if (tag.empty())
				{
					continue;
				}

				if (args.empty())
				{
					continue;
				}

				Vec3 transfrom;
				int maxIndx = glm::min((int)args.size(), 3);
				for (int i = 0; i < maxIndx; i++)
				{
					transfrom[i] = (float)std::atof(args[i].c_str());
				}

				if (tag == "r")
				{
					Quaternion qx = glm::angleAxis(glm::radians(transfrom.x), X_AXIS);
					Quaternion qy = glm::angleAxis(glm::radians(transfrom.y), Y_AXIS);
					Quaternion qz = glm::angleAxis(glm::radians(transfrom.z), Z_AXIS);
					Quaternion q = qz * qy * qx;

					if (set)
					{
						e->m_node->SetOrientation(q, ts);
					}
					else
					{
						e->m_node->Rotate(q, ts);
					}
				}
				else if (tag == "s")
				{
					if (set)
					{
						e->m_node->SetScale(transfrom, ts);
					}
					else
					{
						e->m_node->Scale(transfrom, ts);
					}
				}
				else if (tag == "t")
				{
					if (set)
					{
						e->m_node->SetTranslation(transfrom, ts);
					}
					else
					{
						e->m_node->Translate(transfrom, ts);
					}
				}
			}
		}

		void SetTransformExec(TagArgArray tagArgs)
		{
			TransformInternal(tagArgs, true);
		}

		void TransformExec(TagArgArray tagArgs)
		{
			TransformInternal(tagArgs, false);
		}

		void SetCameraTransformExec(TagArgArray tagArgs)
		{
			TagArgArray::const_iterator viewportTag = GetTag("v", tagArgs);
			if (viewportTag != tagArgs.end())
			{
				if (viewportTag->second.empty()) // Tag cant be empty.
				{
					return;
				}

				if (Viewport* vp = g_app->GetViewport(viewportTag->second[0]))
				{
					if (Camera* c = vp->m_camera)
					{
						assert(c->m_node->m_parent == nullptr && "SetTransform(TS_WORLD) must be implemented.");

						if (viewportTag->second.size() == 2)
						{
							Node* node = c->m_node;
							if (viewportTag->second[1] == "top")
							{
								node->m_orientation = glm::angleAxis(glm::pi<float>(), -Y_AXIS) * glm::angleAxis(glm::half_pi<float>(), X_AXIS) * glm::angleAxis(glm::pi<float>(), Y_AXIS);
							}

							if (viewportTag->second[1] == "bottom")
							{
								node->m_orientation = glm::angleAxis(glm::pi<float>(), -Y_AXIS) * glm::angleAxis(glm::half_pi<float>(), -X_AXIS) * glm::angleAxis(glm::pi<float>(), Y_AXIS);
							}

							if (viewportTag->second[1] == "front")
							{
								node->m_orientation = Quaternion();
							}

							if (viewportTag->second[1] == "back")
							{
								node->m_orientation = glm::angleAxis(glm::pi<float>(), Y_AXIS);
							}

							if (viewportTag->second[1] == "left")
							{
								node->m_orientation = glm::angleAxis(glm::half_pi<float>(), Y_AXIS);
							}

							if (viewportTag->second[1] == "right")
							{
								node->m_orientation = glm::angleAxis(glm::half_pi<float>(), -Y_AXIS);
							}
						}

						TagArgArray::const_iterator translateTag = GetTag("t", tagArgs);
						if (translateTag != tagArgs.end())
						{
							Vec3 translate;
							int maxIndx = glm::min((int)translateTag->second.size(), 3);
							for (int i = 0; i < maxIndx; i++)
							{
								translate[i] = (float)std::atof(translateTag->second[i].c_str());
							}

							c->m_node->m_translation = translate;
						}
					}
				}
			}
		}

		// ImGui ripoff. Portable helpers.
		static int   Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
		static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
		static char* Strdup(const char* str) { size_t len = strlen(str) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)str, len); }
		static void  Strtrim(char* str) { char* str_end = str + strlen(str); while (str_end > str && str_end[-1] == ' ') str_end--; *str_end = 0; }

		ConsoleWindow::ConsoleWindow()
		{
			CreateCommand(g_showPickDebugCmd, ShowPickDebugExec);
			CreateCommand(g_showOverlayUICmd, ShowOverlayExec);
			CreateCommand(g_showOverlayUIAlwaysCmd, ShowOverlayAlwaysExec);
			CreateCommand(g_showModTransitionsCmd, ShowModTransitionsExec);
			CreateCommand(g_setTransformCmd, SetTransformExec);
			CreateCommand(g_setCameraTransformCmd, SetCameraTransformExec);
			CreateCommand(g_transformCmd, TransformExec);
		}

		ConsoleWindow::~ConsoleWindow()
		{
		}

		void ConsoleWindow::Show()
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

		Window::Type ConsoleWindow::GetType()
		{
			return Type::Console;
		}

		void ConsoleWindow::AddLog(const String& log, LogType type)
		{
			String prefixed;
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

		void ConsoleWindow::AddLog(const String& log, const String& tag)
		{
			String prefixed = "[" + tag + "] " + log;
			m_items.push_back(prefixed);
			m_scrollToBottom = true;
		}

		void ConsoleWindow::ClearLog()
		{
			m_items.clear();
		}

		void ConsoleWindow::ExecCommand(const String& commandLine)
		{
			// Split command and args.
			String cmd;
			TagArgArray tagArgs;
			ParseCommandLine(commandLine, cmd, tagArgs);

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
				m_commandExecutors[cmd](tagArgs);
			}
			else
			{
				sprintf_s(buffer, sizeof(buffer), "Unknown command: '%s'\n", cmd.c_str());
				AddLog(buffer, LogType::Error);
			}

			m_scrollToBottom = true;
		}

		void SplitPreserveText(const String& s, const String& sep, std::vector<String>& v)
		{
			String arg = s;
			const char spaceSub = (char)26;

			size_t indx = arg.find_first_of('"');
			size_t indx2 = arg.find_first_of('"', indx + 1);
			while (indx != String::npos && indx2 != String::npos)
			{
				std::replace(arg.begin() + indx, arg.begin() + indx2, ' ', spaceSub);
				std::swap(indx, indx2);
				indx2 = arg.find_first_of('"', indx + 1);
			}

			Split(arg, sep, v);

			for (String& val : v)
			{
				val.erase
				(
					std::remove_if(val.begin(), val.end(), [](char c) { return c == '"'; }),
					val.end()
				);

				std::replace(val.begin(), val.end(), spaceSub, ' ');
			}
		}

		void ConsoleWindow::ParseCommandLine(String commandLine, String& command, TagArgArray& tagArgs)
		{
			String args;
			size_t indx = commandLine.find_first_of(" ");
			if (indx != String::npos)
			{
				args = commandLine.substr(indx + 1);
				command = commandLine.substr(0, indx);
			}
			else
			{
				command = commandLine;
				return;
			}
			
			// Single arg, no tag.
			if (args.find_first_of("--") == String::npos)
			{
				TagArg nullTag;
				std::vector<String> values;
				SplitPreserveText(args, " ", nullTag.second);
				tagArgs.push_back(nullTag);
				return;
			}

			std::vector<String> splits;
			Split(args, "--", splits);

			// Preserve all spaces in text.
			const char spaceSub = (char)26;
			for (String& arg : splits)
			{
				std::vector<String> values;
				SplitPreserveText(arg, " ", values);
				if (values.empty())
				{
					continue;
				}

				String tag;
				tag = values.front();
				pop_front(values);

				if (values.empty())
				{
					continue;
				}

				TagArg input(tag, values);
				tagArgs.push_back(input);
			}
		}

		// Mostly ripoff from the ImGui Console Example.
		int ConsoleWindow::TextEditCallback(ImGuiInputTextCallbackData* data)
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
				std::vector<String> candidates;
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

		void ConsoleWindow::CreateCommand(const String& command, std::function<void(TagArgArray)> executor)
		{
			m_commands.push_back(command);
			m_commandExecutors[command] = executor;
		}

	}
}
