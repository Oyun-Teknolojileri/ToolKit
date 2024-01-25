/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Pass.h"

namespace ToolKit
{

  struct ForwardRenderPassParams
  {
    CameraPtr Cam                   = nullptr;
    FramebufferPtr FrameBuffer      = nullptr;
    FramebufferPtr gFrameBuffer     = nullptr;
    RenderTargetPtr gNormalRt       = nullptr;
    RenderTargetPtr gLinearRt       = nullptr;
    RenderTargetPtr SsaoTexture     = nullptr;
    GraphicBitFields clearBuffer    = GraphicBitFields::AllBits;
    bool SSAOEnabled                = false;
    RenderJobArray* OpaqueJobs      = nullptr;
    RenderJobArray* TranslucentJobs = nullptr;
    LightPtrArray Lights            = {}; //!< Updated lights.
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
    void RenderOpaque(RenderJobArray& jobs, CameraPtr cam, LightPtrArray& lights);

    /**
     * Sorts and renders translucent entities. For double-sided blended entities
     * first render back, than renders front.
     * @param entities All entities to render.
     * @param cam Camera for rendering.
     * @param lights ights All lights.
     */
    void RenderTranslucent(RenderJobArray& jobs, CameraPtr cam, LightPtrArray& lights);

   public:
    ForwardRenderPassParams m_params;
  };

  typedef std::shared_ptr<ForwardRenderPass> ForwardRenderPassPtr;
} // namespace ToolKit