#pragma once

#include "Pass.h"

namespace ToolKit
{

  struct ForwardRenderPassParams
  {
    Camera* Cam                    = nullptr;
    FramebufferPtr FrameBuffer     = nullptr;
    bool ClearFrameBuffer          = true;
    RenderJobArray OpaqueJobs      = {};
    RenderJobArray TranslucentJobs = {};
    LightRawPtrArray Lights        = {};
  };

  /**
   * Renders given entities with given lights using forward rendering
   */
  class TK_API ForwardRenderPass : public RenderPass
  {
   public:
    ForwardRenderPass();
    explicit ForwardRenderPass(const ForwardRenderPassParams& params);
    ~ForwardRenderPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   protected:
    /**
     * Renders the entities immediately. No sorting applied.
     * @param entities All entities to render.
     * @param cam Camera for rendering.
     * @param zoom Zoom amount of camera.
     * @param lights All lights.
     */
    void RenderOpaque(RenderJobArray jobs,
                      Camera* cam,
                      const LightRawPtrArray& lights);

    /**
     * Sorts and renders translucent entities. For double-sided blended entities
     * first render back, than renders front.
     * @param entities All entities to render.
     * @param cam Camera for rendering.
     * @param lights ights All lights.
     */
    void RenderTranslucent(RenderJobArray jobs,
                           Camera* cam,
                           const LightRawPtrArray& lights);

   public:
    ForwardRenderPassParams m_params;
  };

  typedef std::shared_ptr<ForwardRenderPass> ForwardRenderPassPtr;

} // namespace ToolKit