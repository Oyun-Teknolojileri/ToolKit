#include "stdafx.h"
#include "OverlayMenu.h"
#include "Viewport.h"
#include "GlobalDef.h"

ToolKit::Editor::OverlayNav::OverlayNav(Viewport* owner)
{
	m_owner = owner;
}

void ToolKit::Editor::OverlayNav::ShowOverlayNav()
{
	ImVec2 overlaySize(300, 100);
	ImVec2 padding = ImGui::GetStyle().WindowPadding;
	const float headerSize = 17.0f;
	ImVec2 window_pos = ImVec2(m_owner->m_wndPos.x + m_owner->m_wndContentAreaSize.x - overlaySize.x + padding.x, m_owner->m_wndPos.y + headerSize + padding.y);
	ImGui::SetNextWindowPos(window_pos);
	ImGui::SetNextWindowBgAlpha(0.65f);
	if (ImGui::BeginChildFrame(ImGui::GetID("Navigation"), overlaySize, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGuiStyle& style = ImGui::GetStyle();
		float spacing = style.ItemInnerSpacing.x;

		// Select button.
		static float hoverTimeX = 0.0f;
		ImGui::Button("X"); ImGui::SameLine(); EditorGUI::HelpMarker("Camera speed m/s\n", &hoverTimeX); ImGui::SameLine(0, spacing);

		static float hoverTimeY = 0.0f;
		ImGui::Button("Y"); ImGui::SameLine(); EditorGUI::HelpMarker("Camera speed m/s\n", &hoverTimeY); ImGui::SameLine(0, spacing);

		static float hoverTimeZ = 0.0f;
		ImGui::Button("Z"); ImGui::SameLine(); EditorGUI::HelpMarker("Camera speed m/s\n", &hoverTimeZ); ImGui::SameLine(0, spacing);

		const char* items[] = { "0.5", "1", "2", "4", "8" };
		static int current_item = 4;
		ImGui::PushItemWidth(50);
		if (ImGui::BeginCombo("##CS", items[current_item], ImGuiComboFlags_None))
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (current_item == n);
				if (ImGui::Selectable(items[n], is_selected))
				{
					current_item = n;
				}

				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();

		switch (current_item)
		{
		case 0:
			g_app->m_camSpeed = 0.5;
			break;
		case 1:
			g_app->m_camSpeed = 1;
			break;
		case 2:
			g_app->m_camSpeed = 2;
			break;
		case 3:
			g_app->m_camSpeed = 4;
			break;
		case 4:
			g_app->m_camSpeed = 8;
			break;
		default:
			g_app->m_camSpeed = 4;
			break;
		}

		static float hoverTimeCS = 0.0f;
		ImGui::SameLine(0, spacing); EditorGUI::HelpMarker("Camera speed m/s\n", &hoverTimeCS);
	}
	ImGui::EndChildFrame();
}
