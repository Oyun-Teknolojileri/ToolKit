#include "stdafx.h"

#include "ConsoleWindow.h"
#include "GlobalDef.h"

void ToolKit::Editor::ConsoleWindow::Show()
{
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Once);
	ImGui::Begin(g_consoleStr.c_str(), &m_open, ImGuiWindowFlags_NoSavedSettings);
	{

	}
}
