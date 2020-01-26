#include "Viewport.h"
#include "Directional.h"
#include "Renderer.h"

using namespace ToolKit;

uint Editor::Viewport::m_nextId = 1;

Editor::Viewport::Viewport(float width, float height)
	: m_width(width), m_height(height), m_name("Viewport")
{
	m_camera = new Camera();
	m_camera->SetLens(glm::half_pi<float>(), width, height);
	m_viewportImage = new RenderTarget((uint)width, (uint)height);
	m_name += " " + std::to_string(m_nextId++);
}

Editor::Viewport::~Viewport()
{
	SafeDel(m_camera);
	SafeDel(m_viewportImage);
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

void Editor::Viewport::ShowViewport()
{
	ImGui::Begin(m_name.c_str());
	{
		ImGui::SetWindowSize(ImVec2(m_width, m_height), ImGuiCond_FirstUseEver);

		ImVec2 currSize = ImGui::GetWindowSize();
		if (currSize.x != m_width || currSize.y != m_height)
		{
			OnResize(currSize.x, currSize.y);
		}

		ImGui::Image((void*)(intptr_t)m_viewportImage->m_textureId, ImVec2(m_width, m_height));
	}
	ImGui::End();
}
