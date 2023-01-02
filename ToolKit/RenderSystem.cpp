#include "RenderSystem.h"

#include "ToolKit.h"

#include "DebugNew.h"

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

  RenderSystem::~RenderSystem() { SafeDel(m_renderer); }

  void RenderSystem::AddRenderTask(Technique* technique)
  {
    AddRenderTask({[technique](Renderer* renderer) -> void
                   { technique->Render(renderer); }});
  }

  void RenderSystem::AddRenderTask(TechniquePtr technique)
  {
    AddRenderTask({[technique](Renderer* renderer) -> void
                   { technique->Render(renderer); }});
  }

  void RenderSystem::AddRenderTask(RenderTask task)
  {
    m_renderTaskArray.push_back(task);
  }

  void RenderSystem::ExecuteRenderTasks()
  {
    std::vector<RenderTask> tasks = std::move(m_renderTaskArray);
    for (RenderTask rt : tasks)
    {
      if (rt.Task != nullptr)
      {
        rt.Task(m_renderer);

        if (rt.Callback != nullptr)
        {
          rt.Callback();
        }
      }
    }
  }

  void RenderSystem::SetAppWindowSize(uint width, uint height)
  {
    m_renderer->m_windowSize = UVec2(width, height);
  }

  UVec2 RenderSystem::GetAppWindowSize() { return m_renderer->m_windowSize; }

  void RenderSystem::SetClearColor(const Vec4& clearColor)
  {
    m_renderer->m_clearColor = clearColor;
  }

  void RenderSystem::SetFrameCount(uint count)
  {
    m_renderer->m_frameCount = count;
  }

  void RenderSystem::EnableBlending(bool enable)
  {
    m_renderer->EnableBlending(enable);
  }

} // namespace ToolKit
