#include "Viewport.h"
#include "Directional.h"
#include "Renderer.h"
#include "App.h"
#include "GlobalDef.h"
#include "SDL.h"

using namespace ToolKit;

uint Editor::Viewport::m_nextId = 1;

Editor::Viewport::Viewport(float width, float height)
	: m_width(width), m_height(height), m_name("Viewport")
{
	m_camera = new Camera();
	m_camera->SetLens(glm::half_pi<float>(), width, height);
	m_viewportImage = new RenderTarget((uint)width, (uint)height);
	m_viewportImage->Init();
	m_name += " " + std::to_string(m_nextId++);
}

Editor::Viewport::~Viewport()
{
	SafeDel(m_camera);
	SafeDel(m_viewportImage);
}

void Editor::Viewport::Update(uint deltaTime)
{
	UpdateFpsNavigation(deltaTime);
}

void Editor::Viewport::ShowViewport()
{
	ImGui::SetNextWindowSize(ImVec2(m_width, m_height), ImGuiCond_Once);
	ImGui::Begin(m_name.c_str(), &m_open, ImGuiWindowFlags_NoSavedSettings);
	{
		// Content area size
		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();

		vMin.x += ImGui::GetWindowPos().x;
		vMin.y += ImGui::GetWindowPos().y;
		vMax.x += ImGui::GetWindowPos().x;
		vMax.y += ImGui::GetWindowPos().y;

		ImVec2 contentSize(glm::abs(vMax.x - vMin.x), glm::abs(vMax.y - vMin.y));

		if (!ImGui::IsWindowCollapsed())
		{
			if (contentSize.x > 0 && contentSize.y > 0)
			{
				ImGui::Image((void*)(intptr_t)m_viewportImage->m_textureId, ImVec2(m_width, m_height));

				ImVec2 currSize = ImGui::GetContentRegionMax();
				if (contentSize.x != m_width || contentSize.y != m_height)
				{
					OnResize(contentSize.x, contentSize.y);
				}

				ImGui::GetWindowDrawList()->AddRect(vMin, vMax, IM_COL32(128, 128, 128, 255));
			}
		}
	}
	ImGui::End();
}

void Editor::Viewport::OnResize(float width, float height)
{
	m_width = width;
	m_height = height;
	m_camera->SetLens(glm::half_pi<float>(), width, height);

	m_viewportImage->UnInit();
	m_viewportImage->m_width = (uint)width;
	m_viewportImage->m_height = (uint)height;
	m_viewportImage->Init();
}

void Editor::Viewport::UpdateFpsNavigation(uint deltaTime)
{
	if (m_camera)
	{
		ImGuiIO& io = ImGui::GetIO();
		
		// Mouse is rightclicked
		if (ImGui::IsMouseDown(1))
		{
			m_camera->Yaw(glm::radians(-io.MouseDelta.x * g_app->m_mouseSensitivity));
			m_camera->RotateOnUpVector(glm::radians(io.MouseDelta.y * g_app->m_mouseSensitivity));
			

			glm::vec3 dir, up, right;
			m_camera->GetLocalAxis(dir, up, right);

			dir = ToolKit::Z_AXIS;
			up = ToolKit::Y_AXIS;
			right = T;

			float speed = g_app->m_camSpeed;

			glm::vec3 move;
			if (io.KeysDown[SDL_SCANCODE_A])
			{
				move += -right;
			}

			if (io.KeysDown[SDL_SCANCODE_D])
			{
				move += right;
			}

			if (io.KeysDown[SDL_SCANCODE_W])
			{
				move += dir;
			}

			if (io.KeysDown[SDL_SCANCODE_S])
			{
				move += -dir;
			}

			if (io.KeysDown[SDL_SCANCODE_PAGEUP])
			{
				move += up;
			}

			if (io.KeysDown[SDL_SCANCODE_PAGEDOWN])
			{
				move += -up;
			}

			float displace = speed * MilisecToSec(deltaTime);
			m_camera->Translate(move * displace);
		}
	} 
}
