/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "DataTexture.h"
#include "PostProcessPass.h"

namespace ToolKit
{

  class SSAONoiseTexture : public DataTexture
  {
   public:
    TKDeclareClass(SSAONoiseTexture, DataTexture);

    SSAONoiseTexture();
    using DataTexture::NativeConstruct;

    void Init(void* data);

   private:
    void Init(bool flushClientSideArray = false) override;
  };

  typedef std::shared_ptr<SSAONoiseTexture> SSAONoiseTexturePtr;

  struct SSAOPassParams
  {
    TexturePtr GNormalBuffer      = nullptr;
    TexturePtr GLinearDepthBuffer = nullptr;
    CameraPtr Cam                 = nullptr;
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

    int KernelSize                = 64;
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

    FramebufferPtr m_ssaoFramebuffer        = nullptr;
    SSAONoiseTexturePtr m_noiseTexture      = nullptr;
    RenderTargetPtr m_tempBlurRt            = nullptr;

    FullQuadPassPtr m_quadPass              = nullptr;
    ShaderPtr m_ssaoShader                  = nullptr;

    int m_currentKernelSize                 = 0;

    const int m_minimumKernelSize           = 8;
    const int m_maximumKernelSize           = 128;

    // Used to detect if the spread has changed. If so, kernel updated.
    float m_prevSpread                      = -1.0f;

    static StringArray m_ssaoSamplesStrCache;
    static constexpr int m_ssaoSamplesStrCacheSize = 128;
  };

  typedef std::shared_ptr<SSAOPass> SSAOPassPtr;

} // namespace ToolKit