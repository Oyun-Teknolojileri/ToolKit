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
						for (size_t i = 0; i < m_entiries.size(); i++)
						{
							ImGuiStyle& style = ImGui::GetStyle();
							float visX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

							String name;
							DecomposePath(m_entiries[i], nullptr, &name, nullptr);
							
							ImVec2 tSize = ImGui::CalcTextSize(name.c_str());
							if (tSize.x > buttonSz.x * 0.8)
							{
								name = name.substr(0, 4);
								name += "...";
							}

							ImGui::Button(name.c_str(), buttonSz);
							if (ImGui::IsItemHovered())
							{
								ImGui::SetTooltip(m_entiries[i].c_str());
							}

							if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
							{
								ImGui::SetDragDropPayload("DND_DEMO_CELL", &i, sizeof(size_t));    // Set payload to carry the index of our item (could be anything)
								ImGui::Text("Copy %s", m_entiries[i].c_str()); // Display preview (could be anything, e.g. when dragging an image we could decide to display the filename and a small preview of the image, etc.)
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


