#pragma once

#include "PostProcessPass.h"

namespace ToolKit
{

  struct SSAOPassParams
  {
    TexturePtr GPositionBuffer    = nullptr;
    TexturePtr GNormalBuffer      = nullptr;
    TexturePtr GLinearDepthBuffer = nullptr;
    Camera* Cam                   = nullptr;

    /**
     * How far the samples will be taken from.
     */
    float Radius                  = 0.5f;

    /**
     * Base offset from the sample location.
     */
    float Bias                    = 0.025f;

    /**
     * 0-1 value defining how diverse the samples from the normal.
     */
    float spread                  = 1.0;
  };

  class TK_API SSAOPass : public Pass
  {
   public:
    SSAOPass();
    explicit SSAOPass(const SSAOPassParams& params);
    ~SSAOPass();

    void Render();
    void PreRender();
    void PostRender();

   private:
    void GenerateSSAONoise();

   public:
    SSAOPassParams m_params;
    RenderTargetPtr m_ssaoTexture = nullptr;

   private:
    Vec3Array m_ssaoKernel;
    Vec2Array m_ssaoNoise;

    FramebufferPtr m_ssaoFramebuffer   = nullptr;
    SSAONoiseTexturePtr m_noiseTexture = nullptr;
    RenderTargetPtr m_tempBlurRt       = nullptr;

    FullQuadPassPtr m_quadPass         = nullptr;
    ShaderPtr m_ssaoShader             = nullptr;

    // Used to detect if the spread has changed. If so, kernel updated.
    float m_prevSpread                 = -1.0f;
  };

  typedef std::shared_ptr<SSAOPass> SSAOPassPtr;

} // namespace ToolKit