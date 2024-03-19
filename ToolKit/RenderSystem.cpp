/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "RenderSystem.h"

#include "GlErrorReporter.h"
#include "Logger.h"
#include "RHI.h"
#include "TKOpenGL.h"
#include "TKStats.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{
  RenderPath::RenderPath() {}

  RenderPath::~RenderPath() { m_passArray.clear(); }

  void RenderPath::Render(Renderer* renderer)
  {
    for (PassPtr& pass : m_passArray)
    {
      pass->SetRenderer(renderer);
      pass->PreRender();
      pass->Render();
      pass->PostRender();
    }
  }

  RenderSystem::RenderSystem()
  {
    m_renderer    = new Renderer();
    m_renderUtils = new RenderUtils(m_renderer);
  }

  RenderSystem::~RenderSystem()
  {
    SafeDel(m_renderer);
    SafeDel(m_renderUtils);
  }

  void RenderSystem::Init()
  {
    m_renderer->Init();
    AddRenderTask({[](Renderer* renderer) -> void { renderer->GenerateBRDFLutTexture(); }});
  }

  void RenderSystem::AddRenderTask(RenderPath* technique)
  {
    AddRenderTask({[technique](Renderer* renderer) -> void { technique->Render(renderer); }});
  }

  void RenderSystem::AddRenderTask(TechniquePtr technique)
  {
    AddRenderTask({[technique](Renderer* renderer) -> void { technique->Render(renderer); }});
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
    if (m_renderer)
    {
      m_renderer->m_gpuProgramHasFrameUpdates.clear();
    }

    // Immediate execution.
    RenderTaskArray tasks = std::move(m_highQueue);
    for (RenderTask& rt : tasks)
    {
      ExecuteTaskImp(rt);
    }

    // Time limited execution.
    if (!m_lowQueue.empty())
    {
      const float timeLimit = 10.0f; // Adjust this to give more chance.
      float time0           = GetElapsedMilliSeconds();
      float time1           = time0;

      tasks                 = std::move(m_lowQueue);
      while (time1 - time0 < timeLimit && !tasks.empty())
      {
        RenderTask rt = tasks.front();
        pop_front<RenderTask>(tasks);

        ExecuteTaskImp(rt);

        time1 = GetElapsedMilliSeconds();
      }

      // Merge remaining.
      m_lowQueue.insert(m_lowQueue.begin(), tasks.begin(), tasks.end());
      TK_LOG("Asnyc Render %d", m_lowQueue.size());
    }
  }

  void RenderSystem::FlushRenderTasks()
  {
    auto flushTasksFn = [this](RenderTaskArray& rts) -> void
    {
      while (!rts.empty())
      {
        // Complete all existing and potential fallow-up render tasks.
        // For this reason while loop must stay.
        RenderTaskArray tasks = std::move(rts);
        for (RenderTask& rt : tasks)
        {
          ExecuteTaskImp(rt);
        }
      }
    };

    flushTasksFn(m_highQueue);
    flushTasksFn(m_lowQueue);

    m_renderer->m_sky = nullptr;
  }

  void RenderSystem::FlushGpuPrograms() { GetGpuProgramManager()->FlushPrograms(); }

  void RenderSystem::SetAppWindowSize(uint width, uint height) { m_renderer->m_windowSize = UVec2(width, height); }

  UVec2 RenderSystem::GetAppWindowSize() { return m_renderer->m_windowSize; }

  void RenderSystem::SetClearColor(const Vec4& clearColor) { m_renderer->m_clearColor = clearColor; }

  void RenderSystem::EnableBlending(bool enable) { m_renderer->EnableBlending(enable); }

  void RenderSystem::DecrementSkipFrame()
  {
    if (m_skipFrames == 0)
    {
      return;
    }
    m_skipFrames--;
  }

  bool RenderSystem::IsSkipFrame() const { return m_skipFrames != 0; }

  uint RenderSystem::GetFrameCount() { return m_frameCount; }

  void RenderSystem::ResetFrameCount() { m_frameCount = 0; }

  void RenderSystem::SkipSceneFrames(int numFrames) { m_skipFrames = numFrames; }

  void RenderSystem::InitGl(void* glGetProcAddres, GlReportCallback callback)
  {
    // Initialize opengl functions.

#ifdef _WIN32
    gladLoadGLES2((GLADloadfunc) glGetProcAddres);
#endif

    InitGLErrorReport(callback);
    TestSRGBBackBuffer();

    // Default states.
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
  }

  void RenderSystem::ExecuteTaskImp(RenderTask& task)
  {
    if (task.Task != nullptr)
    {
      task.Task(m_renderer);

      if (task.Callback != nullptr)
      {
        task.Callback();
      }
    }
  }

  void RenderSystem::EndFrame()
  {
    m_frameCount++;
    m_renderer->m_frameCount = m_frameCount;
  }

  void RenderSystem::TestSRGBBackBuffer()
  {
    RHI::SetFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.5f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    GLubyte pixel[4];
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    m_backbufferFormatIsSRGB = (pixel[0] > 150);
  }

  RenderUtils* RenderSystem::GetRenderUtils() { return m_renderUtils; }

} // namespace ToolKit
