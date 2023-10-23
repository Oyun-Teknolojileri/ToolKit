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
#include "EngineSettings.h"
#include "ForwardPass.h"
#include "ForwardPreProcessPass.h"
#include "FxaaPass.h"
#include "GBufferPass.h"
#include "GammaPass.h"
#include "Pass.h"
#include "RenderSystem.h"
#include "ShadowPass.h"
#include "SsaoPass.h"
#include "ForwardPreProcessPass.h"
#include "AdditiveLightingPass.h"

namespace ToolKit
{
  struct SceneRenderPathParams
  {
    LightPtrArray Lights;
    ScenePtr Scene                 = nullptr;
    CameraPtr Cam                  = nullptr;
    FramebufferPtr MainFramebuffer = nullptr;
    bool ClearFramebuffer          = true;
    EngineSettings::PostProcessingSettings Gfx;
  };

  /**
   * Main scene renderer.
   */
  class TK_API SceneRenderPath : public RenderPath
  {
   public:
    SceneRenderPath();
    explicit SceneRenderPath(const SceneRenderPathParams& params);
    virtual ~SceneRenderPath();

    virtual void Render(Renderer* renderer) override;
    virtual void PreRender(Renderer* renderer);
    virtual void PostRender();

   private:
    virtual void SetPassParams();

   public:
    SceneRenderPathParams m_params;

   public:
    ShadowPassPtr m_shadowPass                         = nullptr;
    ForwardRenderPassPtr m_forwardRenderPass           = nullptr;
    ForwardPreProcessPassPtr m_forwardPreProcessPass   = nullptr;
    LightingPassPtr m_lightingPass                     = nullptr;
    CubeMapPassPtr m_skyPass                           = nullptr;
    GBufferPassPtr m_gBufferPass                       = nullptr;
    SSAOPassPtr m_ssaoPass                             = nullptr;
    FXAAPassPtr m_fxaaPass                             = nullptr;
    GammaPassPtr m_gammaPass                           = nullptr;
    BloomPassPtr m_bloomPass                           = nullptr;
    TonemapPassPtr m_tonemapPass                       = nullptr;
    DoFPassPtr m_dofPass                               = nullptr;
    LightPtrArray m_updatedLights;

   protected:
    bool m_drawSky   = false;
    SkyBasePtr m_sky = nullptr;
  };

  typedef std::shared_ptr<SceneRenderPath> SceneRenderPathPtr;
} // namespace ToolKit
