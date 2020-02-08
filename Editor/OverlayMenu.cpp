#include "stdafx.h"
#include "OverlayMenu.h"
#include "Viewport.h"

ToolKit::Editor::OverlayNav::OverlayNav(Viewport* owner)
{
	m_owner = owner;
}

void ToolKit::Editor::OverlayNav::ShowOverlayNav()
{
	const float DISTANCE = 50.0f;
	static int corner = 0;
	ImGuiIO& io = ImGui::GetIO();

	ImVec2 window_pos = ImVec2(m_owner->m_lastWndPos.x + DISTANCE, m_owner->m_lastWndPos.y + DISTANCE);
	ImGui::SetNextWindowPos(window_pos);
	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
	if (ImGui::BeginChildFrame(ImGui::GetID("Example: Simple overlay"), ImVec2(200,100), (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGui::Text("Simple overlay\n" "in the corner of the screen.\n" "(right-click to change position)");
		ImGui::Separator();
		if (ImGui::IsMousePosValid())
			ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
		else
			ImGui::Text("Mouse Position: <invalid>");
	}
	ImGui::EndChildFrame();
}
