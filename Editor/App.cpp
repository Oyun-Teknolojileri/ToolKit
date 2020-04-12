#include "stdafx.h"

#include "App.h"
#include "Renderer.h"
#include "UI.h"
#include "Viewport.h"
#include "Primative.h"
#include "Node.h"
#include "GlobalDef.h"
#include "OverlayMenu.h"
#include "Grid.h"
#include "Directional.h"
#include "Mod.h"
#include "ConsoleWindow.h"
#include "Gizmo.h"
#include "DebugNew.h"

namespace ToolKit
{
	namespace Editor
	{

		App::App(int windowWidth, int windowHeight)
		{
			m_suzanne = nullptr;
			m_q1 = nullptr;
			m_q2 = nullptr;
			m_q3 = nullptr;
			m_cursor = nullptr;
			m_renderer = new Renderer();
			m_renderer->m_windowWidth = windowWidth;
			m_renderer->m_windowHeight = windowHeight;
		}

		App::~App()
		{
			// UI.
			for (Window* wnd : m_windows)
			{
				SafeDel(wnd);
			}
			SafeDel(Viewport::m_overlayNav);

			// Editor objects.
			SafeDel(m_grid);
			SafeDel(m_origin);
			SafeDel(m_suzanne);
			SafeDel(m_q1);
			SafeDel(m_q2);
			SafeDel(m_q3);
			SafeDel(m_cursor);

			ModManager::GetInstance()->UnInit();

			// Engine components.
			SafeDel(m_renderer);
			Main::GetInstance()->Uninit();
		}

		void App::Init()
		{
			std::shared_ptr<Material> solidColorMaterial = Main::GetInstance()->m_materialManager.Create(MaterialPath("solidColor.material"));
			solidColorMaterial->m_color = glm::vec3(0.8f, 0.8f, 0.8f);

			m_suzanne = new Drawable();
			m_suzanne->m_mesh = Main::GetInstance()->m_meshMan.Create(MeshPath("suzanne.mesh"));
			m_suzanne->m_mesh->Init(false);
			m_scene.AddEntity(m_suzanne);

			m_cursor = new Cursor();

			m_q1 = new Cube();
			m_q1->m_mesh->Init(false);
			m_q1->m_node->m_translation = glm::vec3(-4.0f, 0.0f, 0.0f);
			m_q1->m_node->m_orientation = glm::angleAxis(1.1f, glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)));
			m_scene.AddEntity(m_q1);

			m_q2 = new Cube();
			m_q2->m_mesh->Init(false);
			m_q2->m_node->m_translation = glm::vec3(4.0f, 0.0f, 0.0f);
			m_scene.AddEntity(m_q2);

			m_q3 = new Cone();
			m_q3->m_mesh->Init(false);
			m_q3->m_node->m_scale = glm::vec3(0.3f, 1.0f, 0.3f);
			m_q3->m_node->m_translation = glm::vec3(2.0f, 0.0f, 0.0f);
			m_scene.AddEntity(m_q3);

			m_origin = new Axis3d();

			m_grid = new Grid(100);
			m_grid->m_mesh->Init(false);

			m_highLightMaterial = std::shared_ptr<Material>(solidColorMaterial->GetCopy());
			m_highLightMaterial->m_color = g_selectHighLightPrimaryColor;
			m_highLightMaterial->GetRenderState()->cullMode = CullingType::Front;

			m_highLightSecondaryMaterial = std::shared_ptr<Material>(solidColorMaterial->GetCopy());
			m_highLightSecondaryMaterial->m_color = g_selectHighLightSecondaryColor;
			m_highLightSecondaryMaterial->GetRenderState()->cullMode = CullingType::Front;

			ModManager::GetInstance()->Init();

			// UI.
			Viewport* vp = new Viewport(m_renderer->m_windowWidth * 0.8f, m_renderer->m_windowHeight * 0.8f);
			m_windows.push_back(vp);

			ConsoleWindow* console = new ConsoleWindow();
			m_windows.push_back(console);

			UI::InitIcons();
		}

		void App::Frame(float deltaTime)
		{
			// Update Mods.
			UI::DispatchSignals();
			ModManager::GetInstance()->Update(deltaTime);

			// Update Viewports.
			for (Window* wnd : m_windows)
			{
				if (wnd->GetType() != Window::Type::Viewport)
				{
					continue;
				}

				Viewport* vp = static_cast<Viewport*> (wnd);
				vp->Update(deltaTime);

				m_renderer->SetRenderTarget(vp->m_viewportImage);

				// Dirty hack.
				MoveGizmo* gizmo = nullptr;

				for (Entity* ntt : m_scene.GetEntities())
				{
					if (ntt->IsDrawable())
					{
						if (ntt->GetType() == EntityType::Entity_Billboard)
						{
							Billboard* billboard = static_cast<Billboard*> (ntt);
							billboard->LookAt(vp->m_camera);

							if (typeid(ntt) == typeid(MoveGizmo))
							{
								gizmo = static_cast<MoveGizmo*> (ntt);
								continue;
							}
						}

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

				m_renderer->Render(m_grid, vp->m_camera);

				m_origin->LookAt(vp->m_camera);
				m_renderer->Render(m_origin, vp->m_camera);

				if (gizmo != nullptr)
				{
					m_renderer->Render(gizmo, vp->m_camera);
				}

				m_cursor->LookAt(vp->m_camera);
				m_renderer->Render(m_cursor, vp->m_camera);
			}

			m_renderer->SetRenderTarget(nullptr);

			// Render UI.
			UI::ShowUI();
		}

		void App::OnResize(int width, int height)
		{
			m_renderer->m_windowWidth = width;
			m_renderer->m_windowHeight = height;
			glViewport(0, 0, width, height);
		}

		void App::OnQuit()
		{
			g_running = false;
		}

		Viewport* App::GetActiveViewport()
		{
			for (Window* wnd : m_windows)
			{
				if (wnd->GetType() != Window::Type::Viewport)
				{
					continue;
				}

				if (wnd->IsActive() && wnd->IsVisible())
				{
					return static_cast<Viewport*> (wnd);
				}
			}

			return nullptr;
		}

		ConsoleWindow* App::GetConsole()
		{
			for (Window* wnd : m_windows)
			{
				if (wnd->GetType() == Window::Type::Console)
				{
					return static_cast<ConsoleWindow*> (wnd);
				}
			}

			return nullptr;
		}

		void App::RenderSelected(Drawable* e, Camera* c)
		{
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDepthMask(GL_FALSE);

			CullingType cm = e->m_mesh->m_material->GetRenderState()->cullMode;
			e->m_mesh->m_material->GetRenderState()->cullMode = CullingType::TwoSided;
			m_renderer->Render(e, c);

			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDepthMask(GL_TRUE);

			e->m_mesh->m_material->GetRenderState()->cullMode = cm;
			m_renderer->Render(e, c);

			std::shared_ptr<Material> m = e->m_mesh->m_material;
			if (m_scene.IsCurrentSelection(e->m_id))
			{
				e->m_mesh->m_material = m_highLightMaterial;
			}
			else
			{
				e->m_mesh->m_material = m_highLightSecondaryMaterial;
			}

			glm::vec3 s = e->m_node->m_scale;
			float dist = glm::distance(e->m_node->GetTranslation(TransformationSpace::TS_WORLD), c->m_node->GetTranslation(TransformationSpace::TS_WORLD));
			e->m_node->m_scale += 0.005f * dist;

			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			m_renderer->Render(e, c);
			glDisable(GL_STENCIL_TEST);

			e->m_node->m_scale = s;
			e->m_mesh->m_material = m;
		}

	}
}
