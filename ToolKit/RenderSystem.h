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
    virtual void PreRender(Renderer* renderer);
    virtual void PostRender(Renderer* renderer);

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
     */
    void SetAppWindowSize(uint width, uint height);

    /** Application window size. */
    UVec2 GetAppWindowSize();

    /** Sets default clear color for render targets. */
    void SetClearColor(const Vec4& clearColor);

    /** Internally used. Enables blending. This should not be used directly. */
    void EnableBlending(bool enable);

    /** Returns elapsed frame count. */
    uint GetFrameCount();

    /** Set elapsed frame count to zero. */
    void ResetFrameCount();

    /** Internally used to decrement skip count. */
    void DecrementSkipFrame();

    /** States if this state is skipped. */
    bool IsSkipFrame() const;

    /** Sets number of frames to skip. */
    void SkipSceneFrames(int numFrames);

    /**
     * Host application must provide opengl function addresses. This function
     * initialize opengl functions.
     * @param glGetProcAddress is the address of opengl function getter.
     * @param callback is error callback function for opengl.
     */
    void InitGl(void* glGetProcAddres, GlReportCallback callback = nullptr);

    /** This function should be called by the end of the frame. */
    void EndFrame();

    /** Returns true if back buffer is not srgb. */
    bool IsGammaCorrectionNeeded() { return !m_backbufferFormatIsSRGB; }

    /** Checks if the backbuffer is srgb and sets m_backbufferFormatIsSRGB. */
    void TestSRGBBackBuffer();

    bool ConsumeGPULightCacheInvalidation();
    void InvalidateGPULightCache();

   private:
    /** Implementation for executing render tasks. */
    void ExecuteTaskImp(RenderTask& task);

   private:
    /** High priority render queue. Tasks in this queue always finished. */
    RenderTaskArray m_highQueue;

    /** Low priority render queue. Consume tasks for a given time span and left others for next frame. */
    RenderTaskArray m_lowQueue;

    /** Current Renderer. */
    Renderer* m_renderer            = nullptr;

    /** Holds number of frames to skip. If its greater than zero renderer skip given frames. */
    int m_skipFrames                = 0;

    /** States if the back buffer is srgb. */
    bool m_backbufferFormatIsSRGB   = true;

    /** Number of elapsed frames since the engine start. */
    uint m_frameCount               = 0;

    /** Consumed by ShadowPass to understands if the shadow atlas should be recreated. */
    bool m_shadowAtlasInvalidated   = false;

    /** Consumed by Renderer to understands if the light cache should be updated. */
    bool m_gpuLightCacheInvalidated = false;
  };

} // namespace ToolKit