/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "GpuProgram.h"
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
    void AddRenderTask(RenderTask task);
    void ExecuteRenderTasks();
    void FlushRenderTasks();
    void FlushGpuPrograms();

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

    uint GetFrameCount();
    void ResetFrameCount();

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

    /**
     * This function should be called by the end of the frame
     */
    void EndFrame();

    inline bool IsGammaCorrectionNeeded() { return !m_backbufferFormatIsSRGB; }

    void TestSRGBBackBuffer();

    bool ConsumeShadowAtlasInvalidation();
    void InvalidateShadowAtlas();

   private:
    void ExecuteTaskImp(RenderTask& task);

   private:
    RenderTaskArray m_highQueue;
    RenderTaskArray m_lowQueue;
    Renderer* m_renderer          = nullptr;
    int m_skipFrames              = 0;
    bool m_backbufferFormatIsSRGB = true;
    uint m_frameCount             = 0;
    bool m_shadowAtlasInvalidated = false;
  };

} // namespace ToolKit