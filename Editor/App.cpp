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
	m_suzanne = nullptr;
	m_q1 = nullptr;
	m_q2 = nullptr;
	m_cursor = nullptr;
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
	SafeDel(m_suzanne);
	SafeDel(m_q1);
	SafeDel(m_q2);
	SafeDel(m_hitMarker);
	SafeDel(m_cursor);
	SafeDel(m_renderer);

	Main::GetInstance()->Uninit();
}

void ToolKit::Editor::App::Init()
{
	std::shared_ptr<Material> solidColorMaterial = ToolKit::Main::GetInstance()->m_materialManager.Create(ToolKit::MaterialPath("solidColor.material"));
	solidColorMaterial->m_color = glm::vec3(0.8f, 0.8f, 0.8f);

	m_suzanne = new Drawable();
	m_suzanne->m_mesh = Main::GetInstance()->m_meshMan.Create(MeshPath("suzanne.mesh"));
	m_suzanne->m_mesh->Init(false);
	m_scene.m_entitites.push_back(m_suzanne);

	m_cursor = new Cursor();

	m_q1 = new Cube();
	m_q1->m_mesh->Init(false);
	m_q1->m_node->m_translation = glm::vec3(-4.0f, 0.0f, 0.0f);
	m_scene.m_entitites.push_back(m_q1);
	m_q2 = new Cube();
	m_q2->m_mesh->Init(false);
	m_q2->m_node->m_translation = glm::vec3(4.0f, 0.0f, 0.0f);
	m_scene.m_entitites.push_back(m_q2);

	m_hitMarker = new Sphere();
	m_hitMarker->m_mesh->m_material = std::shared_ptr<Material>(solidColorMaterial->GetCopy());
	m_hitMarker->m_mesh->m_material->m_color = glm::vec3(1.0f, 0.627f, 0.156f);
	m_hitMarker->m_node->m_scale = glm::vec3(0.03f, 0.03f, 0.03f);
	m_scene.m_entitites.push_back(m_hitMarker);

	m_origin = new Axis3d();
	m_scene.m_entitites.push_back(m_origin);

	m_grid = new Grid(100);
	m_grid->m_mesh->Init(false);
	m_scene.m_entitites.push_back(m_grid);

	m_highLightMaterial = std::shared_ptr<Material>(solidColorMaterial->GetCopy());
	m_highLightMaterial->m_color = glm::vec3(1.0f, 0.627f, 0.156f);
	m_highLightMaterial->GetRenderState()->cullMode = CullingType::Front;

	m_highLightSecondaryMaterial = std::shared_ptr<Material>(solidColorMaterial->GetCopy());
	m_highLightSecondaryMaterial->m_color = glm::vec3(0.898f, 0.352f, 0.031f);
	m_highLightSecondaryMaterial->GetRenderState()->cullMode = CullingType::Front;

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
				if (m_scene.IsSelected(ntt->m_id))
				{
					RenderSelected((Drawable*)ntt, vp->m_camera);
				}
				else
				{
					m_renderer->Render((Drawable*)ntt, vp->m_camera);
				}
			}
		}

		m_cursor->LookAt(vp->m_camera);
		m_renderer->Render(m_cursor, vp->m_camera);
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

void ToolKit::Editor::App::RenderSelected(Drawable* e, Camera* c)
{
	glm::vec3 s = e->m_node->m_scale;
	e->m_node->m_scale += 0.02f;

	std::shared_ptr<Material> m = e->m_mesh->m_material;
	if (m_scene.IsCurrentSelection(e->m_id))
	{
		e->m_mesh->m_material = m_highLightMaterial;
	}
	else
	{
		e->m_mesh->m_material = m_highLightSecondaryMaterial;
	}

	m_renderer->Render(e, c);

	e->m_node->m_scale = s;
	e->m_mesh->m_material = m;
	m_renderer->Render(e, c);
}

ToolKit::Editor::Scene::PickData ToolKit::Editor::Scene::PickObject(Ray ray, const std::vector<EntityId>& ignoreList)
{
	PickData pd;
	float closestPickedDistance = FLT_MAX;
	for (Entity* e : m_entitites)
	{
		if (!e->IsDrawable())
		{
			continue;
		}

		if (std::find(ignoreList.begin(), ignoreList.end(), e->m_id) != ignoreList.end())
		{
			continue;
		}

		Ray rayInObjectSpace = ray;
		glm::mat4 modelTs = e->m_node->GetTransform();
		glm::mat4 InvModelTs = glm::inverse(modelTs);
		rayInObjectSpace.position = InvModelTs * glm::vec4(ray.position, 1.0f);
		rayInObjectSpace.direction = glm::transpose(modelTs) * glm::vec4(ray.direction, 1.0f);

		float dist = 0;
		Drawable* dw = static_cast<Drawable*>(e);
		if (RayBoxIntersection(rayInObjectSpace, dw->m_mesh->m_AABoundingBox, dist))
		{
			if (RayMeshIntersection(dw->m_mesh.get(), rayInObjectSpace, dist))
			{
				if (dist < closestPickedDistance && dist > 0.0f)
				{
					pd.entity = e;
					pd.pickPos = ray.position + ray.direction * dist;
					closestPickedDistance = dist;
				}
			}
		}
	}

	return pd;
}

bool ToolKit::Editor::Scene::IsSelected(EntityId id)
{
	return std::find(m_selectedEntities.begin(), m_selectedEntities.end(), id) != m_selectedEntities.end();
}

void ToolKit::Editor::Scene::RemoveFromSelection(EntityId id)
{
	auto nttIt = std::find(m_selectedEntities.begin(), m_selectedEntities.end(), id);
	if (nttIt != m_selectedEntities.end())
	{
		m_selectedEntities.erase(nttIt);
	}
}

void ToolKit::Editor::Scene::AddToSelection(EntityId id)
{
	assert(!IsSelected(id));
	m_selectedEntities.push_back(id);
}

void ToolKit::Editor::Scene::ClearSelection()
{
	m_selectedEntities.clear();
}

bool ToolKit::Editor::Scene::IsCurrentSelection(EntityId id)
{
	return m_selectedEntities.back() == id;
}
