/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "AdditiveLightingPass.h"
#include "BloomPass.h"
#include "CubemapPass.h"
#include "EngineSettings.h"
#include "ForwardPass.h"
#include "ForwardPreProcessPass.h"
#include "GBufferPass.h"
#include "Pass.h"
#include "RenderSystem.h"
#include "ShadowPass.h"
#include "SsaoPass.h"

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
  class TK_API DeferredSceneRenderPath : public RenderPath
  {
   public:
    DeferredSceneRenderPath();
    explicit DeferredSceneRenderPath(const SceneRenderPathParams& params);
    virtual ~DeferredSceneRenderPath();

    virtual void Render(Renderer* renderer) override;
    virtual void PreRender(Renderer* renderer);
    virtual void PostRender(Renderer* renderer);

   private:
    virtual void SetPassParams();

   public:
    SceneRenderPathParams m_params;

   public:
    ShadowPassPtr m_shadowPass                       = nullptr;
    ForwardRenderPassPtr m_forwardRenderPass         = nullptr;
    ForwardPreProcessPassPtr m_forwardPreProcessPass = nullptr;
    LightingPassPtr m_lightingPass                   = nullptr;
    CubeMapPassPtr m_skyPass                         = nullptr;
    GBufferPassPtr m_gBufferPass                     = nullptr;
    SSAOPassPtr m_ssaoPass                           = nullptr;
    BloomPassPtr m_bloomPass                         = nullptr;
    DoFPassPtr m_dofPass                             = nullptr;

   protected:
    bool m_drawSky   = false;
    SkyBasePtr m_sky = nullptr;

    // Cached variables
    RenderData m_renderData;
  };

  typedef std::shared_ptr<DeferredSceneRenderPath> SceneRenderPathPtr;
} // namespace ToolKit
