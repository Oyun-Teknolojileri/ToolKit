#pragma once

#include "Pass.h"
#include "Renderer.h"

#include <functional>

namespace ToolKit
{

  /**
   * Base class responsible of creating render results using passes.
   */
  class TK_API Technique
  {
   public:
    Technique();
    virtual ~Technique();
    virtual void Render(Renderer* renderer);

   public:
    PassPtrArray m_passArray;
  };

  typedef std::shared_ptr<Technique> TechniquePtr;

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
    void AddRenderTask(Technique* technique);
    void AddRenderTask(TechniquePtr technique);
    void AddRenderTask(RenderTask task);
    void ExecuteRenderTasks();
    void FlushRenderTasks();

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

   private:
    void ExecuteTaskImp(RenderTask& task);

   private:
    RenderTaskArray m_highQueue;
    RenderTaskArray m_lowQueue;
    Renderer* m_renderer         = nullptr;
    Technique* m_renderTechnique = nullptr;
  };

} // namespace ToolKit