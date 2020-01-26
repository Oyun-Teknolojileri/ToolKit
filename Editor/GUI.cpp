#include "App.h"
#include "GUI.h"
#include "Viewport.h"

extern SDL_Window* g_window;
extern Editor::App* g_app;

void Editor::EditorGUI::PresentGUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(g_window);
	ImGui::NewFrame();

	InitDocking();
	ShowAppMainMenuBar();

	for (Viewport* vp : g_app->m_viewports)
	{
		vp->ShowViewport();
	}

	ShowSimpleWindow();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::EditorGUI::ShowSimpleWindow()
{
	ImGui::Begin("Another Window");
	ImGui::Text("Hello from another window!");
	ImGui::Button("Close Me");
	ImGui::End();
}

void Editor::EditorGUI::InitDocking()
{
	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", nullptr, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	ImGui::End();
}

void Editor::EditorGUI::ShowAppMainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ShowMenuFile();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Windows"))
		{
			ShowMenuWindows();
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void Editor::EditorGUI::ShowMenuFile()
{
	if (ImGui::MenuItem("Quit", "Alt+F4"))
	{
		g_app->OnQuit();
	}
}

void Editor::EditorGUI::ShowMenuWindows()
{
	if (ImGui::MenuItem("Add Viewport", "Alt+V"))
	{
		Viewport* vp = new Viewport(640, 480);
		g_app->m_viewports.push_back(vp);
	}
}
