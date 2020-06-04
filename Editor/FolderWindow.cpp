#include "stdafx.h"

#include "FolderWindow.h"
#include "GlobalDef.h"
#include "DebugNew.h"

#include <filesystem>

namespace ToolKit
{
	namespace Editor
	{

		FolderWindow::FolderWindow()
		{
		}

		void FolderWindow::Show()
		{
			ImGui::Begin("AssetBrowser", &m_visible);
			{
				if (ImGui::BeginTabBar("Folders", ImGuiTabBarFlags_None))
				{
					if (ImGui::BeginTabItem("FNameHere"))
					{
						ImVec2 buttonSz(50, 50);
						for (int i = 0; i < (int)m_entiries.size(); i++)
						{
							ImGuiStyle& style = ImGui::GetStyle();
							float visX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

							String name, ext;
							DecomposePath(m_entiries[i], nullptr, &name, &ext);

							uint iconId = UI::m_fileIcon->m_textureId;
							if (ext == MESH)
							{
								iconId = UI::m_meshIcon->m_textureId;
							}
							else if (ext == ANIM)
							{
								iconId = UI::m_clipIcon->m_textureId;
							}
							else if (ext == SKINMESH)
							{
								iconId = UI::m_armatureIcon->m_textureId;
							}
							else if (ext == AUDIO)
							{
								iconId = UI::m_audioIcon->m_textureId;
							}
							else if (ext == SHADER)
							{
								iconId = UI::m_codeIcon->m_textureId;
							}
							else if (ext == SKELETON)
							{
								iconId = UI::m_boneIcon->m_textureId;
							}

							ImGui::PushID(i);
							ImGui::BeginGroup();
							ImGui::ImageButton((void*)(intptr_t)iconId, buttonSz);
							if (ImGui::IsItemHovered())
							{
								ImGui::SetTooltip(m_entiries[i].c_str());
							}

							ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + buttonSz.x);
							ImGui::TextWrapped(name.c_str());
							ImGui::PopTextWrapPos();

							if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
							{
								ImGui::SetDragDropPayload("DND_DEMO_CELL", &i, sizeof(size_t));
								ImGui::Text("Copy %s", m_entiries[i].c_str());
								ImGui::EndDragDropSource();
							}

							if (ImGui::BeginDragDropTarget())
							{
								if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
								{
									IM_ASSERT(payload->DataSize == sizeof(size_t));
									int payload_n = *(const int*)payload->Data;
								}
								ImGui::EndDragDropTarget();
							}
							ImGui::EndGroup();
							ImGui::PopID();

							float lastBtnX2 = ImGui::GetItemRectMax().x;
							float nextBtnX2 = lastBtnX2 + style.ItemSpacing.x + buttonSz.x;
							if (nextBtnX2 < visX2)
							{
								ImGui::SameLine();
							}
						}
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
				ImGui::End();
			}
		}

		Window::Type FolderWindow::GetType()
		{
			return Window::Type::Browser;
		}

		void FolderWindow::Iterate(const String& path)
		{
			using namespace std::filesystem;

			for (const directory_entry& e : directory_iterator(path))
			{
				String name, ext;
				DecomposePath(e.path().u8string(), nullptr, &name, &ext);
				m_entiries.push_back(name + ext);
			}
		}

	}
}
