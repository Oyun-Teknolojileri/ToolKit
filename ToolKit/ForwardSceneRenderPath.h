/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "BloomPass.h"
#include "CubemapPass.h"
#include "EngineSettings.h"
#include "ForwardPass.h"
#include "ForwardPreProcessPass.h"
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
   * Forward scene render path. All objects are drawn in forward manner. Bandwidth optimized.
   */
  class TK_API ForwardSceneRenderPath : public RenderPath
  {
   public:
    ForwardSceneRenderPath();
    explicit ForwardSceneRenderPath(const SceneRenderPathParams& params);
    virtual ~ForwardSceneRenderPath();

    void Render(Renderer* renderer) override;
    void PreRender(Renderer* renderer) override;
    void PostRender(Renderer* renderer) override;

   protected:
    void SetPassParams();
    bool RequiresForwardPreProcessPass();

   public:
    SceneRenderPathParams m_params;

   public:
    ShadowPassPtr m_shadowPass                       = nullptr;
    ForwardRenderPassPtr m_forwardRenderPass         = nullptr;
    ForwardPreProcessPassPtr m_forwardPreProcessPass = nullptr;
    CubeMapPassPtr m_skyPass                         = nullptr;
    SSAOPassPtr m_ssaoPass                           = nullptr;
    BloomPassPtr m_bloomPass                         = nullptr;
    DoFPassPtr m_dofPass                             = nullptr;

   protected:
    bool m_drawSky   = false;
    SkyBasePtr m_sky = nullptr;

    // Cached variables
    RenderData m_renderData;
  };

  typedef std::shared_ptr<ForwardSceneRenderPath> SceneRenderPathPtr;

} // namespace ToolKit
