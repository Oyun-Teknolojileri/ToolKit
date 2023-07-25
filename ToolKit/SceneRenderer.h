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

#include "BloomPass.h"
#include "CubemapPass.h"
#include "DeferredPass.h"
#include "EngineSettings.h"
#include "ForwardPass.h"
#include "FxaaPass.h"
#include "GBufferPass.h"
#include "GammaPass.h"
#include "Pass.h"
#include "RenderSystem.h"
#include "ShadowPass.h"
#include "SsaoPass.h"
#include "ForwardLinearDepthPass.h"

namespace ToolKit
{

  struct SceneRenderPassParams
  {
    LightRawPtrArray Lights;
    ScenePtr Scene                 = nullptr;
    Camera* Cam                    = nullptr;
    FramebufferPtr MainFramebuffer = nullptr;
    bool ClearFramebuffer          = true;
    EngineSettings::PostProcessingSettings Gfx;
  };

  /**
   * Main scene renderer.
   */
  class TK_API SceneRenderer : public Technique
  {
   public:
    SceneRenderer();
    explicit SceneRenderer(const SceneRenderPassParams& params);
    virtual ~SceneRenderer();

    void Render(Renderer* renderer) override;
    void PreRender(Renderer* renderer);
    void PostRender();

   private:
    void SetPassParams();

   public:
    SceneRenderPassParams m_params;

   public:
    ShadowPassPtr m_shadowPass                         = nullptr;
    ForwardRenderPassPtr m_forwardRenderPass           = nullptr;
    ForwardLinearDepthPassPtr m_forwardPreProcessPass = nullptr;
    CubeMapPassPtr m_skyPass                           = nullptr;
    GBufferPassPtr m_gBufferPass                       = nullptr;
    DeferredRenderPassPtr m_deferredRenderPass         = nullptr;
    SSAOPassPtr m_ssaoPass                             = nullptr;
    FXAAPassPtr m_fxaaPass                             = nullptr;
    GammaPassPtr m_gammaPass                           = nullptr;
    BloomPassPtr m_bloomPass                           = nullptr;
    TonemapPassPtr m_tonemapPass                       = nullptr;
    DoFPassPtr m_dofPass                               = nullptr;
    LightRawPtrArray m_updatedLights;

   private:
    bool m_drawSky = false;
    SkyBase* m_sky = nullptr;
  };

  typedef std::shared_ptr<SceneRenderer> SceneRendererPtr;
} // namespace ToolKit