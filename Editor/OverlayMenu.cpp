#include "stdafx.h"
#include "OverlayMenu.h"
#include "Viewport.h"
#include "GlobalDef.h"
#include "Mod.h"

ToolKit::Editor::OverlayNav::OverlayNav(Viewport* owner)
{
	m_owner = owner;
}

void ToolKit::Editor::OverlayNav::ShowOverlayNav()
{
	ImVec2 overlaySize(48, 258);
	ImVec2 padding = ImGui::GetStyle().WindowPadding;
	const float headerSize = 17.0f;
	ImVec2 window_pos = ImVec2(m_owner->m_wndPos.x + padding.x + 5, m_owner->m_wndPos.y + headerSize + padding.y + 5);
	ImGui::SetNextWindowPos(window_pos);
	ImGui::SetNextWindowBgAlpha(0.65f);
	if (ImGui::BeginChildFrame(ImGui::GetID("Navigation"), overlaySize, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGuiStyle& style = ImGui::GetStyle();
		float spacing = style.ItemInnerSpacing.x;

		// Select button.
		static float hoverTimeSelectBtn = 0.0f;
		bool isCurrentMod = ModManager::GetInstance()->m_modStack.back()->m_id == ModId::Base;
		if (isCurrentMod)
		{
			ImGui::PushID(1);
			ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonHovered]);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_ButtonHovered]);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.Colors[ImGuiCol_ButtonHovered]);
		}
		ImGui::ImageButton((void*)(intptr_t)EditorGUI::m_selectIcn->m_textureId, ImVec2(32, 32));
		if (isCurrentMod)
		{
			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}

		EditorGUI::HelpMarker("Select Box\nSelect items using box selection.", &hoverTimeSelectBtn);

		// Cursor button.
		static float hoverTimeCursorBtn = 0.0f;
		ImGui::ImageButton((void*)(intptr_t)EditorGUI::m_cursorIcn->m_textureId, ImVec2(32, 32));
		EditorGUI::HelpMarker("Cursor\nSet the cursor location. Drag to transform.", &hoverTimeCursorBtn);
		ImGui::Separator();

		// Move button.
		static float hoverTimeMoveBtn = 0.0f;
		ImGui::ImageButton((void*)(intptr_t)EditorGUI::m_moveIcn->m_textureId, ImVec2(32, 32));
		EditorGUI::HelpMarker("Move\nMove selected items.", &hoverTimeMoveBtn);

		// Rotate button.
		static float hoverTimeRotateBtn = 0.0f;
		ImGui::ImageButton((void*)(intptr_t)EditorGUI::m_rotateIcn->m_textureId, ImVec2(32, 32));
		EditorGUI::HelpMarker("Rotate\nRotate selected items.", &hoverTimeRotateBtn);

		// Scale button.
		static float hoverTimeScaleBtn = 0.0f;
		ImGui::ImageButton((void*)(intptr_t)EditorGUI::m_scaleIcn->m_textureId, ImVec2(32, 32));
		EditorGUI::HelpMarker("Scale\nScale (resize) selected items.", &hoverTimeScaleBtn);
		ImGui::Separator();

		const char* items[] = {"1", "2", "4", "8" };
		static int current_item = 2;
		ImGui::PushItemWidth(40);
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
