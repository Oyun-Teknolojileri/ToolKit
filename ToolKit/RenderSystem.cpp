/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "RenderSystem.h"

#include "GlErrorReporter.h"
#include "Logger.h"
#include "TKOpenGL.h"
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

  RenderSystem::RenderSystem() { m_renderer = new Renderer(); }

  RenderSystem::~RenderSystem() { SafeDel(m_renderer); }

  void RenderSystem::Init() { m_renderer->Init(); }

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
      GetLogger()->WriteConsole(LogType::Warning, "Asnyc Render %d", m_lowQueue.size());
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

  void RenderSystem::SetAppWindowSize(uint width, uint height) { m_renderer->m_windowSize = UVec2(width, height); }

  UVec2 RenderSystem::GetAppWindowSize() { return m_renderer->m_windowSize; }

  void RenderSystem::SetClearColor(const Vec4& clearColor) { m_renderer->m_clearColor = clearColor; }

  void RenderSystem::SetFrameCount(uint count) { m_renderer->m_frameCount = count; }

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

  void RenderSystem::TestSRGBBackBuffer()
  {
    GLint lastFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.5f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    GLubyte pixel[4];
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    m_backbufferFormatIsSRGB = (pixel[0] > 150);

    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
  }

} // namespace ToolKit
