#include "App.h"

#include "Renderer.h"
#include "GUI.h"
#include "Viewport.h"
#include "Primative.h"
#include "Node.h"

extern bool g_running;

Editor::App::App(int windowWidth, int windowHeight)
{
	m_dummy = nullptr;
	m_renderer = new Renderer();
	m_renderer->m_windowWidth = windowWidth;
	m_renderer->m_windowHeight = windowHeight;
}

Editor::App::~App()
{
	for (Viewport* vp : m_viewports)
	{
		SafeDel(vp);
	}

	SafeDel(m_dummy);
	SafeDel(m_renderer);

	Main::GetInstance()->Uninit();
}

void Editor::App::Init()
{
	Main::GetInstance()->Init();

	m_dummy = new Cube();
	m_dummy->m_mesh->Init();
	m_dummy->m_node->Rotate(glm::angleAxis(glm::radians(45.f), ToolKit::X_AXIS));
	m_dummy->m_node->Rotate(glm::angleAxis(glm::radians(45.f), ToolKit::Y_AXIS));
	m_dummy->m_node->Rotate(glm::angleAxis(glm::radians(45.f), ToolKit::Z_AXIS));
	m_dummy->m_node->Translate(glm::vec3(0.0f, 0.0f, -2.0f));
	m_scene.m_entitites.push_back(m_dummy);
}

void Editor::App::Frame(int deltaTime)
{
	m_dummy->m_node->Rotate(glm::angleAxis(glm::radians(1.f), ToolKit::Z_AXIS));

	// Update Viewports
	for (int i = (int)m_viewports.size() - 1; i >= 0; i--)
	{
		Viewport* vp = m_viewports[i];
		if (!vp->m_open)
		{
			SafeDel(vp);
			m_viewports.erase(m_viewports.begin() + i);
		}
	}

	for (Viewport* vp : m_viewports)
	{
		m_renderer->SetRenderTarget(vp->m_viewportImage);

		for (Entity* ntt : m_scene.m_entitites)
		{
			if (ntt->IsDrawable())
			{
				m_renderer->Render((Drawable*)ntt, vp->m_camera);
			}
		}
	}

	m_renderer->SetRenderTarget(nullptr);

	// Render Gui
	EditorGUI::PresentGUI();
}

void Editor::App::OnResize(int width, int height)
{
	m_renderer->m_windowWidth = width;
	m_renderer->m_windowHeight = height;
	glViewport(0, 0, width, height);
}

void Editor::App::OnQuit()
{
	g_running = false;
}
