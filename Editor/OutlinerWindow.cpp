#include "stdafx.h"
#include "OutlinerWindow.h"
#include "GlobalDef.h"

namespace ToolKit
{
	namespace Editor
	{

		OutlinerWindow::OutlinerWindow()
		{

		}

		// Recursively show entity hierarchy.
		void ShowNode(Entity* e)
		{
			static ImGuiTreeNodeFlags baseFlags 
				= ImGuiTreeNodeFlags_OpenOnArrow 
				| ImGuiTreeNodeFlags_OpenOnDoubleClick 
				| ImGuiTreeNodeFlags_SpanAvailWidth;

			ImGuiTreeNodeFlags nodeFlags = baseFlags;
			if (g_app->m_scene.IsSelected(e->m_id))
			{
				nodeFlags |= ImGuiTreeNodeFlags_Selected;
			}

			auto UpdateSelectionFn = [](Entity* e)
			{
				if (ImGui::IsItemClicked())
				{
					if (ImGui::GetIO().KeyCtrl)
					{
						if (g_app->m_scene.IsSelected(e->m_id))
						{
							g_app->m_scene.RemoveFromSelection(e->m_id);
						}
						else
						{
							g_app->m_scene.AddToSelection(e->m_id, true);
						}
					}
					else
					{
						g_app->m_scene.ClearSelection();
						g_app->m_scene.AddToSelection(e->m_id, false);
					}
				}
			};

			if (e->m_node->m_children.empty())
			{
				nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				ImGui::TreeNodeEx(e->m_name.c_str(), nodeFlags);
				UpdateSelectionFn(e);
			}
			else
			{
				for (Node* n : e->m_node->m_children)
				{
					Entity* childNtt = n->m_entity;
					if (childNtt != nullptr)
					{
						if (childNtt->m_node->m_children.empty())
						{
							nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
							ImGui::TreeNodeEx(e->m_name.c_str(), nodeFlags);
							UpdateSelectionFn(e);
						}
						else
						{
							if (ImGui::TreeNodeEx(childNtt->m_name.c_str(), nodeFlags))
							{
								UpdateSelectionFn(e);
								for (Node* deepChildNode : childNtt->m_node->m_children)
								{
									Entity* deepChild = deepChildNode->m_entity;
									if (deepChild)
									{
										ShowNode(deepChild);
									}
								}

								ImGui::TreePop();
							}
						}
					}
				}
			}
		}

		void OutlinerWindow::Show()
		{
			if (ImGui::Begin(m_name.c_str(), &m_visible))
			{
				HandleStates();

				if (ImGui::TreeNode("Scene"))
				{
					const EntityRawPtrArray& ntties = g_app->m_scene.GetEntities();
					EntityRawPtrArray roots;
					GetRootEntities(ntties, roots);

					ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());

					for (Entity* e : roots)
					{
						ShowNode(e);
					}
					ImGui::TreePop();
				}
			}
			ImGui::End();
		}

		Window::Type OutlinerWindow::GetType() const
		{
			return Window::Type::Outliner;
		}

	}
}