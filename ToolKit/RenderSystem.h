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

#pragma once

#include "Pass.h"

namespace ToolKit
{

  /**
   * Base class responsible of creating render results using passes.
   */
  class TK_API RenderPath
  {
   public:
    RenderPath();
    virtual ~RenderPath();
    virtual void Render(Renderer* renderer);

   public:
    PassPtrArray m_passArray;
  };

  typedef std::shared_ptr<RenderPath> TechniquePtr;

  typedef std::function<void(Renderer*)> RenderTaskFn;
  typedef std::function<void()> RenderTaskOnComplatedFn;

  enum class RenderTaskPriority
  {
    High,
    Low
  };

  struct RenderTask
  {
    RenderTaskFn Task                = nullptr;
    RenderTaskOnComplatedFn Callback = nullptr;
    RenderTaskPriority Priority      = RenderTaskPriority::High;
  };

  typedef std::vector<RenderTask> RenderTaskArray;

  /**
   * System class that facilitates renderer to the techniques.
   */
  class TK_API RenderSystem
  {
   public:
    RenderSystem();
    ~RenderSystem();

    void Init();
    void AddRenderTask(RenderPath* technique);
    void AddRenderTask(TechniquePtr technique);
    void AddRenderTask(RenderTask task);
    void ExecuteRenderTasks();
    void FlushRenderTasks();
    Renderer* GetRenderer();

    /**
     * Sets application window size. Doesn't necessarily update any frame buffer
     * or render target. Systems that rely on this data updates them selfs.
     * Programmer is responsible to keep this value up to date when application
     * size has changed.
     * @param width of the Application window.
     * @param height of the Application window.
     */
    void SetAppWindowSize(uint width, uint height);

    /**
     * @return Application window size.
     */
    UVec2 GetAppWindowSize();

    /**
     * Sets default clear color for render targets.
     * @param clearColor default clear color.
     */
    void SetClearColor(const Vec4& clearColor);

    /**
     * Sets frame count to be that will be used as uniform in shaders.
     */
    void SetFrameCount(uint count);

    void EnableBlending(bool enable);

    void DecrementSkipFrame();

    bool IsSkipFrame() const;

    void SkipSceneFrames(int numFrames);

    /**
     * Host application must provide opengl function addresses. This function
     * initialize opengl functions.
     * @param glGetProcAddress is the adress of opengl function getter.
     * @param callback is error callback function for opengl.
     */
    void InitGl(void* glGetProcAddres, GlReportCallback callback = nullptr);

   private:
    void ExecuteTaskImp(RenderTask& task);

   private:
    RenderTaskArray m_highQueue;
    RenderTaskArray m_lowQueue;
    Renderer* m_renderer         = nullptr;
    RenderPath* m_renderTechnique = nullptr;
    int m_skipFrames             = 0;
  };

} // namespace ToolKit