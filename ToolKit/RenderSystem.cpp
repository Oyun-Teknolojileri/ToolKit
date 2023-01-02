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
    switch (task.Priority)
    {
    case RenderTaskPriority::High:
      m_highQueue.push_back(task);
      break;
    case RenderTaskPriority::Low:
      m_lowQueue.push_back(task);
      break;
    }
  }

  void RenderSystem::ExecuteRenderTasks()
  {
    auto executeFn = [this](RenderTask& rt) -> void
    {
      if (rt.Task != nullptr)
      {
        rt.Task(m_renderer);

        if (rt.Callback != nullptr)
        {
          rt.Callback();
        }
      }
    };

    // Immediate execution.
    std::vector<RenderTask> tasks = std::move(m_highQueue);
    for (RenderTask& rt : tasks)
    {
      executeFn(rt);
    }

    // Time limited execution.
    if (!m_lowQueue.empty())
    {
      const float timeLimit = 10.0f; // Adjust this to give more chance.
      float time0           = GetElapsedMilliSeconds();
      float time1           = time0;

      tasks                 = std::move(m_lowQueue);
      while (time1 - time0 < timeLimit)
      {
        RenderTask rt = tasks.front();
        pop_front<RenderTask>(tasks);

        executeFn(rt);

        time1 = GetElapsedMilliSeconds();
      }

      // Merge remaining.
      m_lowQueue.insert(m_lowQueue.begin(), tasks.begin(), tasks.end());
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
