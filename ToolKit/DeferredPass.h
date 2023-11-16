/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "FullQuadPass.h"
#include "Pass.h"

namespace ToolKit
{

  struct DeferredRenderPassParams
  {
    LightPtrArray lights              = {};
    FramebufferPtr MainFramebuffer    = nullptr;
    FramebufferPtr GBufferFramebuffer = nullptr;
    bool ClearFramebuffer             = true;
    CameraPtr Cam                     = nullptr;
    TexturePtr AOTexture              = nullptr;
  };

  class TK_API DeferredRenderPass : public RenderPass
  {
   public:
    DeferredRenderPass();
    DeferredRenderPass(const DeferredRenderPassParams& params);
    ~DeferredRenderPass();

    void PreRender() override;
    void PostRender() override;
    void Render() override;

   private:
    void InitLightDataTexture();

   public:
    DeferredRenderPassParams m_params;

   private:
    FullQuadPassPtr m_fullQuadPass         = nullptr;
    ShaderPtr m_deferredRenderShader       = nullptr;
    LightDataTexturePtr m_lightDataTexture = nullptr;
    const IVec2 m_lightDataTextureSize     = IVec2(1024);
  };

  typedef std::shared_ptr<DeferredRenderPass> DeferredRenderPassPtr;

} // namespace ToolKit