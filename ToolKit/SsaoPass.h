/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
    TexturePtr GPositionBuffer    = nullptr;
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