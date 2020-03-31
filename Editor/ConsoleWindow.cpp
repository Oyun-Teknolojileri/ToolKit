#include "stdafx.h"

#include "ConsoleWindow.h"
#include "GlobalDef.h"

void ToolKit::Editor::ConsoleWindow::Show()
{
	if (!IsOpen())
	{
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Once);
	ImGui::Begin(g_consoleStr.c_str(), &m_open, ImGuiWindowFlags_NoSavedSettings);
	{
		static float f = 0.5f;
		static int n = 0;
		static bool b = true;
		ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
		ImGui::InputFloat("Input", &f, 0.1f);
		ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
		ImGui::Checkbox("Check", &b);
	}
	ImGui::End();
}

void ToolKit::Editor::ConsoleWindow::SetVisibility(bool visible)
{
	m_open = visible;
}

bool ToolKit::Editor::ConsoleWindow::IsOpen()
{
	return m_open;
}
