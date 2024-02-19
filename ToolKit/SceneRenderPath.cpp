/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "SceneRenderPath.h"

#include "Scene.h"
#include "TKProfiler.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{
  SceneRenderPath::SceneRenderPath()
  {
    m_shadowPass            = MakeNewPtr<ShadowPass>();
    m_forwardRenderPass     = MakeNewPtr<ForwardRenderPass>();
    m_forwardPreProcessPass = MakeNewPtr<ForwardPreProcess>();
    m_lightingPass          = MakeNewPtr<AdditiveLightingPass>();
    m_skyPass               = MakeNewPtr<CubeMapPass>();
    m_gBufferPass           = MakeNewPtr<GBufferPass>();
    m_ssaoPass              = MakeNewPtr<SSAOPass>();
    m_tonemapPass           = MakeNewPtr<TonemapPass>();
    m_fxaaPass              = MakeNewPtr<FXAAPass>();
    m_gammaPass             = MakeNewPtr<GammaPass>();
    m_bloomPass             = MakeNewPtr<BloomPass>();
    m_dofPass               = MakeNewPtr<DoFPass>();
  }

  SceneRenderPath::SceneRenderPath(const SceneRenderPathParams& params) : SceneRenderPath() { m_params = params; }

  SceneRenderPath::~SceneRenderPath()
  {
    m_shadowPass        = nullptr;
    m_forwardRenderPass = nullptr;
    m_lightingPass      = nullptr;
    m_skyPass           = nullptr;
    m_gBufferPass       = nullptr;
    m_ssaoPass          = nullptr;
    m_tonemapPass       = nullptr;
    m_fxaaPass          = nullptr;
    m_gammaPass         = nullptr;
    m_bloomPass         = nullptr;
    m_dofPass           = nullptr;
  }

  void SceneRenderPath::Render(Renderer* renderer)
  {
    PUSH_CPU_MARKER("SceneRenderPath::PreRender");
    PreRender(renderer);
    POP_CPU_MARKER();

    PUSH_CPU_MARKER("SceneRenderPath::Render");

    // First stage of the render.
    m_passArray.clear();

    // Set current sky.
    renderer->m_sky = m_sky;

    // Gbuffer for deferred render
    m_passArray.push_back(m_gBufferPass);

    m_passArray.push_back(m_forwardPreProcessPass);

    // SSAO pass
    if (m_params.Gfx.SSAOEnabled)
    {
      m_passArray.push_back(m_ssaoPass);
    }

    // Shadow pass
    m_passArray.push_back(m_shadowPass);

    RenderPath::Render(renderer);

    // Second stage of the render.
    m_passArray.clear();

    renderer->SetShadowAtlas(Cast<Texture>(m_shadowPass->GetShadowAtlas()));

    // Render non-blended entities with deferred renderer
    m_passArray.push_back(m_lightingPass);

    if (m_drawSky)
    {
      m_passArray.push_back(m_skyPass);
    }

    // Forward render blended entities
    m_passArray.push_back(m_forwardRenderPass);

    // Post processes.
    if (m_params.Gfx.BloomEnabled)
    {
      m_passArray.push_back(m_bloomPass);
    }

    if (m_params.Gfx.DepthOfFieldEnabled)
    {
      m_passArray.push_back(m_dofPass);
    }

    if (m_params.Gfx.TonemappingEnabled)
    {
      m_passArray.push_back(m_tonemapPass);
    }

    if (m_params.Gfx.FXAAEnabled)
    {
      m_passArray.push_back(m_fxaaPass);
    }

    if (m_params.Gfx.GammaCorrectionEnabled)
    {
      m_passArray.push_back(m_gammaPass);
    }

    RenderPath::Render(renderer);

    renderer->SetShadowAtlas(nullptr);

    POP_CPU_MARKER();

    PUSH_CPU_MARKER("SceneRenderer::PostRender");
    PostRender();
    POP_CPU_MARKER();
  }

  void SceneRenderPath::PreRender(Renderer* renderer)
  {
    SetPassParams();

    m_gBufferPass->InitGBuffers(m_params.MainFramebuffer->GetSettings().width,
                                m_params.MainFramebuffer->GetSettings().height);

    m_forwardPreProcessPass->InitBuffers(m_params.MainFramebuffer->GetSettings().width,
                                         m_params.MainFramebuffer->GetSettings().height);
  }

  void SceneRenderPath::PostRender() { m_updatedLights.clear(); }

  void SceneRenderPath::SetPassParams()
  {
    CPU_FUNC_RANGE();

    // Update all lights before using them.
    m_updatedLights                   = m_params.Lights.empty() ? m_params.Scene->GetLights() : m_params.Lights;

    const EntityPtrArray& allDrawList = m_params.Scene->GetEntities();

    m_renderData.jobs.clear();

    LightPtrArray nullLights;
    RenderJobProcessor::CreateRenderJobs(allDrawList,
                                         m_renderData.jobs,
                                         nullLights,
                                         m_params.Cam,
                                         m_params.Scene->GetEnvironmentVolumes(),
                                         false);

    m_shadowPass->m_params.shadowVolume = m_params.Scene->m_boundingBox;

    m_shadowPass->m_params.renderData   = &m_renderData;
    m_shadowPass->m_params.Lights       = m_updatedLights;
    m_shadowPass->m_params.ViewCamera   = m_params.Cam;

    RenderJobProcessor::SeperateRenderData(m_renderData, false);
    RenderJobProcessor::StableSortByMeshThanMaterail(m_renderData);

    // Assign lights for forward pass
    RenderJobProcessor::AssignLight(m_renderData.GetForwardTranslucentBegin(),
                                    m_renderData.jobs.end(),
                                    m_updatedLights);

    // TK_LOG("Culled");
    // int i = 0;
    // for (RenderJobItr beg = m_renderData.jobs.begin(); beg != m_renderData.GetDefferedBegin(); beg++)
    //{
    //   i++;
    //   TK_LOG("%d, %s", i, beg->Entity->GetNameVal().c_str());
    // }

    // TK_LOG("Deferred Opaque");
    // for (RenderJobItr beg = m_renderData.GetDefferedBegin(); beg != m_renderData.GetForwardOpaqueBegin(); beg++)
    //{
    //   i++;
    //   TK_LOG("%d, %s", i, beg->Entity->GetNameVal().c_str());
    // }

    // TK_LOG("Forward Opaque");
    // for (RenderJobItr beg = m_renderData.GetForwardOpaqueBegin(); beg != m_renderData.GetForwardTranslucentBegin();
    //      beg++)
    //{
    //   i++;
    //   TK_LOG("%d, %s", i, beg->Entity->GetNameVal().c_str());
    // }

    // TK_LOG("Forward Translucent");
    // for (RenderJobItr beg = m_renderData.GetForwardTranslucentBegin(); beg != m_renderData.jobs.end(); beg++)
    //{
    //   i++;
    //   TK_LOG("%d, %s", i, beg->Entity->GetNameVal().c_str());
    // }

    m_gBufferPass->m_params.renderData          = &m_renderData;
    m_gBufferPass->m_params.Camera              = m_params.Cam;

    m_forwardRenderPass->m_params.renderData    = &m_renderData;
    m_forwardRenderPass->m_params.Lights        = m_updatedLights;
    m_forwardRenderPass->m_params.Cam           = m_params.Cam;
    m_forwardRenderPass->m_params.gNormalRt     = m_gBufferPass->m_gNormalRt;
    m_forwardRenderPass->m_params.gLinearRt     = m_gBufferPass->m_gLinearDepthRt;
    m_forwardRenderPass->m_params.FrameBuffer   = m_params.MainFramebuffer;
    m_forwardRenderPass->m_params.gFrameBuffer  = m_gBufferPass->m_framebuffer;
    m_forwardRenderPass->m_params.SSAOEnabled   = m_params.Gfx.SSAOEnabled;
    m_forwardRenderPass->m_params.SsaoTexture   = m_ssaoPass->m_ssaoTexture;
    m_forwardRenderPass->m_params.clearBuffer   = GraphicBitFields::None;

    m_forwardPreProcessPass->m_params           = m_forwardRenderPass->m_params;

    m_lightingPass->m_params.ClearFramebuffer   = false;
    m_lightingPass->m_params.GBufferFramebuffer = m_gBufferPass->m_framebuffer;
    m_lightingPass->m_params.lights             = m_updatedLights;
    m_lightingPass->m_params.MainFramebuffer    = m_params.MainFramebuffer;
    m_lightingPass->m_params.Cam                = m_params.Cam;
    m_lightingPass->m_params.AOTexture          = m_params.Gfx.SSAOEnabled ? m_ssaoPass->m_ssaoTexture : nullptr;

    m_ssaoPass->m_params.GNormalBuffer          = m_forwardPreProcessPass->m_normalRt;
    m_ssaoPass->m_params.GLinearDepthBuffer     = m_forwardPreProcessPass->m_linearDepthRt;
    m_ssaoPass->m_params.Cam                    = m_params.Cam;
    m_ssaoPass->m_params.Radius                 = m_params.Gfx.SSAORadius;
    m_ssaoPass->m_params.spread                 = m_params.Gfx.SSAOSpread;
    m_ssaoPass->m_params.Bias                   = m_params.Gfx.SSAOBias;
    m_ssaoPass->m_params.KernelSize             = m_params.Gfx.SSAOKernelSize;

    // Set CubeMapPass for sky.
    m_drawSky                                   = false;
    if (m_sky = m_params.Scene->GetSky())
    {
      m_sky->Init();
      if (m_drawSky = m_sky->GetDrawSkyVal())
      {
        if (m_sky->ReadyToRender())
        {
          m_skyPass->m_params.FrameBuffer = m_params.MainFramebuffer;
          m_skyPass->m_params.Cam         = m_params.Cam;
          m_skyPass->m_params.Transform   = m_sky->m_node->GetTransform();
          m_skyPass->m_params.Material    = m_sky->GetSkyboxMaterial();
        }
        else
        {
          m_drawSky = false;
        }
      }
    }

    // Bloom pass
    m_bloomPass->m_params.FrameBuffer    = m_params.MainFramebuffer;
    m_bloomPass->m_params.intensity      = m_params.Gfx.BloomIntensity;
    m_bloomPass->m_params.minThreshold   = m_params.Gfx.BloomThreshold;
    m_bloomPass->m_params.iterationCount = m_params.Gfx.BloomIterationCount;

    // DoF pass
    m_dofPass->m_params.ColorRt    = m_params.MainFramebuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0);
    m_dofPass->m_params.DepthRt    = m_forwardPreProcessPass->m_linearDepthRt;
    m_dofPass->m_params.focusPoint = m_params.Gfx.FocusPoint;
    m_dofPass->m_params.focusScale = m_params.Gfx.FocusScale;
    m_dofPass->m_params.blurQuality     = m_params.Gfx.DofQuality;

    // Tonemap pass.
    m_tonemapPass->m_params.FrameBuffer = m_params.MainFramebuffer;
    m_tonemapPass->m_params.Method      = m_params.Gfx.TonemapperMode;

    // FXAA Pass
    m_fxaaPass->m_params.FrameBuffer    = m_params.MainFramebuffer;
    const FramebufferSettings& fbs      = m_params.MainFramebuffer->GetSettings();
    m_fxaaPass->m_params.screen_size    = Vec2(fbs.width, fbs.height);

    // Gamma pass.
    m_gammaPass->m_params.FrameBuffer   = m_params.MainFramebuffer;
    m_gammaPass->m_params.Gamma         = m_params.Gfx.Gamma;
  }

} // namespace ToolKit
