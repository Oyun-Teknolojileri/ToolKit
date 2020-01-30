#include "stdafx.h"

#include "App.h"
#include "Renderer.h"
#include "GUI.h"
#include "Viewport.h"
#include "Primative.h"
#include "Node.h"
#include "GlobalDef.h"
#include "DebugNew.h"

ToolKit::Editor::App::App(int windowWidth, int windowHeight)
{
	m_dummy = nullptr;
	m_renderer = new Renderer();
	m_renderer->m_windowWidth = windowWidth;
	m_renderer->m_windowHeight = windowHeight;
}

ToolKit::Editor::App::~App()
{
	for (Viewport* vp : m_viewports)
	{
		SafeDel(vp);
	}

	SafeDel(m_dummy);
	SafeDel(m_renderer);

	Main::GetInstance()->Uninit();
}

void ToolKit::Editor::App::Init()
{
	m_dummy = new Drawable();
	m_dummy->m_mesh = Main::GetInstance()->m_meshMan.Create(MeshPath("zemin.mesh"));
	m_dummy->m_mesh->Init();
	m_dummy->m_node->Rotate(glm::angleAxis(glm::radians(180.f), ToolKit::X_AXIS));
	m_scene.m_entitites.push_back(m_dummy);
}

void ToolKit::Editor::App::Frame(int deltaTime)
{
	// Update Viewports
	for (int i = (int)m_viewports.size() - 1; i >= 0; i--)
	{
		Viewport* vp = m_viewports[i];
		vp->Update(deltaTime);

		if (!vp->IsOpen())
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

void ToolKit::Editor::App::OnResize(int width, int height)
{
	m_renderer->m_windowWidth = width;
	m_renderer->m_windowHeight = height;
	glViewport(0, 0, width, height);
}

void ToolKit::Editor::App::OnQuit()
{
	g_running = false;
}
