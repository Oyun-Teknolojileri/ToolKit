#include "stdafx.h"

#include "App.h"
#include "Renderer.h"
#include "GUI.h"
#include "Viewport.h"
#include "Primative.h"
#include "Node.h"
#include "GlobalDef.h"
#include "DebugNew.h"
#include "OverlayMenu.h"
#include "Grid.h"
#include "Directional.h"

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

	SafeDel(Viewport::m_overlayNav);
	SafeDel(m_grid);
	SafeDel(m_origin);
	SafeDel(m_dummy);
	SafeDel(m_renderer);

	Main::GetInstance()->Uninit();
}

void ToolKit::Editor::App::Init()
{
	m_dummy = new Drawable();
	m_dummy->m_mesh = Main::GetInstance()->m_meshMan.Create(MeshPath("zemin.mesh"));
	m_dummy->m_mesh->Init();
	m_scene.m_entitites.push_back(m_dummy);

	m_origin = new Axis3d();
	m_scene.m_entitites.push_back(m_origin);

	m_grid = new Grid(100);
	m_scene.m_entitites.push_back(m_grid);

	Viewport* vp = new Viewport(m_renderer->m_windowWidth * 0.8f, m_renderer->m_windowHeight * 0.8f);
	m_viewports.push_back(vp);

	EditorGUI::InitIcons();
}

void ToolKit::Editor::App::Frame(int deltaTime)
{
	// Update Viewports
	for (int i = (int)m_viewports.size() - 1; i >= 0; i--)
	{
		Viewport* vp = m_viewports[i];
		vp->Update(deltaTime);
		
		// Test Shoot rays. For debug purpose, leaks memory (rayMdl, camRayMdl);
		if (vp->IsViewportQueriable() && ImGui::GetIO().MouseClicked[0])
		{
			static Arrow2d* rayMdl = nullptr;
			static Arrow2d* camRayMdl = nullptr;
			if (rayMdl == nullptr)
			{
				rayMdl = new Arrow2d();
				camRayMdl = new Arrow2d(Arrow2d::ArrowType::Y);
				m_scene.m_entitites.push_back(rayMdl);
				m_scene.m_entitites.push_back(camRayMdl);
			}

			Ray r = vp->RayFromMousePosition();
			rayMdl->m_node->m_translation = r.position;
			rayMdl->m_node->m_orientation = ToolKit::RotationTo(ToolKit::X_AXIS, r.direction);

			glm::vec3 cd, ps;
			vp->m_camera->GetLocalAxis(cd, ps, ps);
			ps = vp->m_camera->m_node->m_translation;
			assert(glm::epsilonEqual(glm::length(cd), 1.0f, 0.0001f));

			camRayMdl->m_mesh->UnInit();
			camRayMdl->m_mesh->m_clientSideVertices.clear();
			Vertex v;
			v.pos = ps;
			camRayMdl->m_mesh->m_clientSideVertices.push_back(v);
			v.pos = v.pos + cd;
			camRayMdl->m_mesh->m_clientSideVertices.push_back(v);
		}

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

ToolKit::Editor::Scene::PickData ToolKit::Editor::Scene::PickObject(Ray ray)
{
	PickData pd;
	float closestPickedDistance = FLT_MAX;
	for (Entity* e : m_entitites)
	{
		if (!e->IsDrawable())
		{
			continue;
		}

		Ray rayInObjectSpace = ray;
		glm::mat4 modelTs = e->m_node->GetTransform();
		glm::mat4 InvModelTs = glm::inverse(modelTs);
		rayInObjectSpace.position = InvModelTs * glm::vec4(ray.position, 1.0f);
		rayInObjectSpace.direction = glm::transpose(modelTs) * glm::vec4(ray.direction, 1.0f);

		Drawable* dw = static_cast<Drawable*>(e);
		if (RayBoxIntersection(rayInObjectSpace, dw->m_mesh->m_AABoundingBox))
		{
			// pd.entity = e;
			glm::vec3 triangle[3];
			std::vector<Mesh*> meshes;
			dw->m_mesh->GetAllMeshes(meshes);
			for (auto mesh : meshes)
			{
				if (dw->m_mesh->m_clientSideIndices.empty())
				{
					
				}
				else
				{

				}
			}
		}
	}

	return pd;
}
