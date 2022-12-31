#include "RenderSystem.h"

#include "stdafx.h"

namespace ToolKit
{
  Technique::Technique() {}

  Technique::~Technique() { m_passArray.clear(); }

  void Technique::Render(Renderer* renderer)
  {
    for (PassPtr& pass : m_passArray)
    {
      pass->SetRenderer(renderer);
      pass->PreRender();
      pass->Render();
      pass->PostRender();
    }
  }

  RenderSystem::RenderSystem() { m_renderer = new Renderer(); }

  RenderSystem::~RenderSystem() {}

  void RenderSystem::Render(Technique* technique)
  {
    technique->Render(m_renderer);
  }

  void RenderSystem::Render(TechniquePtr technique)
  {
    technique->Render(m_renderer);
  }

} // namespace ToolKit
