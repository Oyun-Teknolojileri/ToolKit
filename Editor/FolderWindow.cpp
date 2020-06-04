#include "stdafx.h"

#include "FolderWindow.h"
#include "GlobalDef.h"
#include "DebugNew.h"

#include <filesystem>

namespace ToolKit
{
	namespace Editor
	{

		FolderView::FolderView()
		{
		}

		FolderView::FolderView(class FolderWindow* parent)
		{
			m_parent = parent;
		}

		void FolderView::Show()
		{
			if (ImGui::BeginTabItem(m_folder.c_str(), &m_visible))
			{
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip(m_path.c_str());
				}

				ImVec2 buttonSz(50, 50);
				for (int i = 0; i < (int)m_entiries.size(); i++)
				{
					ImGuiStyle& style = ImGui::GetStyle();
					float visX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

					DirectoryEntry& de = m_entiries[i];

					uint iconId = UI::m_fileIcon->m_textureId;
					if (de.m_isDirectory)
					{
						iconId = UI::m_folderIcon->m_textureId;
					}
					else if (de.m_ext == MESH)
					{
						iconId = UI::m_meshIcon->m_textureId;
					}
					else if (de.m_ext == ANIM)
					{
						iconId = UI::m_clipIcon->m_textureId;
					}
					else if (de.m_ext == SKINMESH)
					{
						iconId = UI::m_armatureIcon->m_textureId;
					}
					else if (de.m_ext == AUDIO)
					{
						iconId = UI::m_audioIcon->m_textureId;
					}
					else if (de.m_ext == SHADER)
					{
						iconId = UI::m_codeIcon->m_textureId;
					}
					else if (de.m_ext == SKELETON)
					{
						iconId = UI::m_boneIcon->m_textureId;
					}
					else if (de.m_ext == MATERIAL)
					{
						iconId = UI::m_materialIcon->m_textureId;
					}
					else if (de.m_ext == PNG)
					{
						iconId = UI::m_imageIcon->m_textureId;
					}
					else if (de.m_ext == JPEG)
					{
						iconId = UI::m_imageIcon->m_textureId;
					}
					else if (de.m_ext == TGA)
					{
						iconId = UI::m_imageIcon->m_textureId;
					}
					else if (de.m_ext == BMP)
					{
						iconId = UI::m_imageIcon->m_textureId;
					}
					else if (de.m_ext == PSD)
					{
						iconId = UI::m_imageIcon->m_textureId;
					}

					ImGui::PushID(i);
					ImGui::BeginGroup();
					ImGui::ImageButton((void*)(intptr_t)iconId, buttonSz);

					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						if (ImGui::IsItemHovered())
						{
							if (de.m_isDirectory)
							{
								if (m_parent != nullptr)
								{
									FolderView view(m_parent);
									view.SetPath(de.m_rootPath + "\\" + de.m_fileName);
									view.Iterate();
									m_parent->AddEntry(view);
								}
							}
						}
					}

					String fullName = de.m_fileName + de.m_ext;
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip(fullName.c_str());
					}

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						ImGui::SetDragDropPayload("BrowserDragZone", &i, sizeof(int));
						ImGui::Text("Copy %s", fullName.c_str());
						ImGui::EndDragDropSource();
					}

					ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + buttonSz.x);
					ImGui::TextWrapped(de.m_fileName.c_str());
					ImGui::PopTextWrapPos();

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BrowserDragZone"))
						{
							IM_ASSERT(payload->DataSize == sizeof(int));
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
		}

		void FolderView::SetPath(const String& path)
		{
			m_path = path;
			StringArray splits;
			Split(path, "\\", splits);
			m_folder = splits.back();
		}

		void FolderView::Iterate()
		{
			using namespace std::filesystem;

			for (const directory_entry& e : directory_iterator(m_path))
			{
				DirectoryEntry de;
				de.m_isDirectory = e.is_directory();
				de.m_rootPath = e.path().parent_path().u8string();
				de.m_fileName = e.path().stem().u8string();
				de.m_ext = e.path().filename().extension().u8string();

				m_entiries.push_back(de);
			}
		}

		FolderWindow::FolderWindow()
		{
		}

		void FolderWindow::Show()
		{
			ImGui::Begin(m_name.c_str(), &m_visible);
			{
				if (ImGui::BeginTabBar("Folders", ImGuiTabBarFlags_NoTooltip))
				{

					for (FolderView& fv : m_entiries)
					{
						fv.Show();
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
				if (e.is_directory())
				{
					FolderView view(this);
					view.SetPath(e.path().u8string());
					view.Iterate();
					m_entiries.push_back(view);
				}
			}
		}

		void FolderWindow::AddEntry(const FolderView& view)
		{
			m_entiries.push_back(view);
		}

	}
}
