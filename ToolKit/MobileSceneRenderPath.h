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

#include "CubemapPass.h"
#include "EngineSettings.h"
#include "ForwardPass.h"
#include "ForwardPreProcessPass.h"
#include "Pass.h"
#include "RenderSystem.h"
#include "ShadowPass.h"
#include "SsaoPass.h"
#include "FxaaPass.h"

namespace ToolKit
{
  struct MobileSceneRenderPathParams
  {
    LightPtrArray Lights;
    ScenePtr Scene                 = nullptr;
    CameraPtr Cam                  = nullptr;
    FramebufferPtr MainFramebuffer = nullptr;
    bool ClearFramebuffer          = true;
    EngineSettings::PostProcessingSettings Gfx;
  };

  /**
   * Mobile scene render path.
   */
  class TK_API MobileSceneRenderPath : public RenderPath
  {
   public:
    MobileSceneRenderPath();
    explicit MobileSceneRenderPath(const MobileSceneRenderPathParams& params);
    virtual ~MobileSceneRenderPath();

    void Render(Renderer* renderer) override;
    void PreRender(Renderer* renderer);
    void PostRender();

   private:
    void SetPassParams();

   public:
    MobileSceneRenderPathParams m_params;

   public:
    ShadowPassPtr m_shadowPass                       = nullptr;
    ForwardRenderPassPtr m_forwardRenderPass         = nullptr;
    CubeMapPassPtr m_skyPass                         = nullptr;
    ForwardPreProcessPassPtr m_forwardPreProcessPass = nullptr;
    SSAOPassPtr m_ssaoPass                           = nullptr;
    FXAAPassPtr m_fxaaPass                           = nullptr;

    LightPtrArray m_updatedLights;

   private:
    bool m_drawSky   = false;
    SkyBasePtr m_sky = nullptr;
  };

  typedef std::shared_ptr<MobileSceneRenderPath> MobileSceneRenderPathPtr;
} // namespace ToolKit
