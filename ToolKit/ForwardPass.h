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
    RenderData* renderData       = nullptr;
    CameraPtr Cam                = nullptr;
    FramebufferPtr FrameBuffer   = nullptr;
    RenderTargetPtr SsaoTexture  = nullptr;
    GraphicBitFields clearBuffer = GraphicBitFields::AllBits;
    bool SSAOEnabled             = false;
    LightPtrArray Lights         = {}; //!< Updated lights.
  };

  /**
   * Renders given entities with given lights using forward rendering
   */
  class TK_API ForwardRenderPass : public RenderPass
  {
   public:
    ForwardRenderPass();
    ForwardRenderPass(const ForwardRenderPassParams& params);
    ~ForwardRenderPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   protected:
    void RenderOpaque(RenderData* renderData);
    void RenderTranslucent(RenderData* renderData);

    void RenderOpaqueHelper(RenderData* renderData,
                            RenderJobItr begin,
                            RenderJobItr end,
                            GpuProgramPtr defaultGpuProgram);

   public:
    ForwardRenderPassParams m_params;
  };

  typedef std::shared_ptr<ForwardRenderPass> ForwardRenderPassPtr;

} // namespace ToolKit