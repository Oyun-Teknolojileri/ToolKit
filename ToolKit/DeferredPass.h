#pragma once

#include "Pass.h"
#include "FullQuadPass.h"

namespace ToolKit
{

  struct DeferredRenderPassParams
  {
    LightRawPtrArray lights;
    FramebufferPtr MainFramebuffer    = nullptr;
    FramebufferPtr GBufferFramebuffer = nullptr;
    bool ClearFramebuffer             = true;
    Camera* Cam                       = nullptr;
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