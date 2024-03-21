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

  struct GBufferPassParams
  {
    RenderData* renderData = nullptr;
    CameraPtr Camera       = nullptr;
  };

  class TK_API GBufferPass : public RenderPass
  {
   public:
    GBufferPass();
    explicit GBufferPass(const GBufferPassParams& params);
    ~GBufferPass();

    void PreRender() override;
    void PostRender() override;
    void Render() override;
    void InitGBuffers(int width, int height);
    void UnInitGBuffers();

   public:
    FramebufferPtr m_framebuffer           = nullptr;
    RenderTargetPtr m_gPosRt               = nullptr;
    RenderTargetPtr m_gNormalRt            = nullptr;
    RenderTargetPtr m_gColorRt             = nullptr;
    RenderTargetPtr m_gEmissiveRt          = nullptr;
    RenderTargetPtr m_gLinearDepthRt       = nullptr;
    RenderTargetPtr m_gMetallicRoughnessRt = nullptr;
    RenderTargetPtr m_gIblRt               = nullptr;

    int m_width                            = 1024;
    int m_height                           = 1024;

    GBufferPassParams m_params;

   private:
    bool m_initialized                       = false;
    MaterialPtr m_gBufferMaterial            = nullptr;
    MaterialPtr m_gBufferAlphaMaskedMaterial = nullptr;
  };

  typedef std::shared_ptr<GBufferPass> GBufferPassPtr;

} // namespace ToolKit