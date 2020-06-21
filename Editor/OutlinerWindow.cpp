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
			if (e->m_node->m_children.empty())
			{
				ImGui::Text(e->m_name.c_str());
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
							ImGui::Text(childNtt->m_name.c_str());
						}
						else
						{
							if (ImGui::TreeNode(childNtt->m_name.c_str()))
							{

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