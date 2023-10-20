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

#include "SceneRenderPath.h"

#include "Scene.h"
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
    PreRender(renderer);

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

    renderer->SetShadowAtlas(std::static_pointer_cast<Texture>(m_shadowPass->GetShadowAtlas()));

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

    PostRender();
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
    // Update all lights before using them.
    m_updatedLights = m_params.Lights.empty() ? m_params.Scene->GetLights() : m_params.Lights;

    for (LightPtr light : m_updatedLights)
    {
      light->UpdateShadowCamera();
    }

    EntityPtrArray allDrawList = m_params.Scene->GetEntities();

    RenderJobArray jobs;
    RenderJobProcessor::CreateRenderJobs(allDrawList, jobs);

    m_shadowPass->m_params.RendeJobs = jobs;
    m_shadowPass->m_params.Lights    = m_updatedLights;

    RenderJobProcessor::CullRenderJobs(jobs, m_params.Cam);

    RenderJobProcessor::AssignEnvironment(jobs, m_params.Scene->GetEnvironmentVolumes());

    RenderJobArray deferred, forward, translucent;
    RenderJobProcessor::SeperateDeferredForward(jobs, deferred, forward, translucent);

    m_gBufferPass->m_params.RendeJobs              = deferred;
    m_gBufferPass->m_params.Camera                 = m_params.Cam;

    m_forwardRenderPass->m_params.Lights           = m_updatedLights;
    m_forwardRenderPass->m_params.Cam              = m_params.Cam;
    m_forwardRenderPass->m_params.gNormalRt        = m_gBufferPass->m_gNormalRt;
    m_forwardRenderPass->m_params.gLinearRt        = m_gBufferPass->m_gLinearDepthRt;
    m_forwardRenderPass->m_params.FrameBuffer      = m_params.MainFramebuffer;
    m_forwardRenderPass->m_params.gFrameBuffer     = m_gBufferPass->m_framebuffer;
    m_forwardRenderPass->m_params.SSAOEnabled      = m_params.Gfx.SSAOEnabled;
    m_forwardRenderPass->m_params.SsaoTexture      = m_ssaoPass->m_ssaoTexture;
    m_forwardRenderPass->m_params.ClearFrameBuffer = false;
    m_forwardRenderPass->m_params.OpaqueJobs       = forward;
    m_forwardRenderPass->m_params.TranslucentJobs  = translucent;

    m_forwardPreProcessPass->m_params              = m_forwardRenderPass->m_params;

    m_lightingPass->m_params.ClearFramebuffer      = false;
    m_lightingPass->m_params.GBufferFramebuffer    = m_gBufferPass->m_framebuffer;
    m_lightingPass->m_params.lights                = m_updatedLights;
    m_lightingPass->m_params.MainFramebuffer       = m_params.MainFramebuffer;
    m_lightingPass->m_params.Cam                   = m_params.Cam;
    m_lightingPass->m_params.AOTexture             = m_params.Gfx.SSAOEnabled ? m_ssaoPass->m_ssaoTexture : nullptr;

    m_ssaoPass->m_params.GNormalBuffer             = m_forwardPreProcessPass->m_normalRt;
    m_ssaoPass->m_params.GLinearDepthBuffer        = m_forwardPreProcessPass->m_linearDepthRt;
    m_ssaoPass->m_params.Cam                       = m_params.Cam;
    m_ssaoPass->m_params.Radius                    = m_params.Gfx.SSAORadius;
    m_ssaoPass->m_params.spread                    = m_params.Gfx.SSAOSpread;
    m_ssaoPass->m_params.Bias                      = m_params.Gfx.SSAOBias;
    m_ssaoPass->m_params.KernelSize                = m_params.Gfx.SSAOKernelSize;

    // Set CubeMapPass for sky.
    m_drawSky                                      = false;
    if (m_sky = m_params.Scene->GetSky())
    {
      if (m_drawSky = m_sky->GetDrawSkyVal())
      {
        m_skyPass->m_params.FrameBuffer = m_params.MainFramebuffer;
        m_skyPass->m_params.Cam         = m_params.Cam;
        m_skyPass->m_params.Transform   = m_sky->m_node->GetTransform();
        m_skyPass->m_params.Material    = m_sky->GetSkyboxMaterial();
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
    FramebufferSettings fbs             = m_params.MainFramebuffer->GetSettings();
    m_fxaaPass->m_params.screen_size    = Vec2(fbs.width, fbs.height);

    // Gamma pass.
    m_gammaPass->m_params.FrameBuffer   = m_params.MainFramebuffer;
    m_gammaPass->m_params.Gamma         = m_params.Gfx.Gamma;
  }

} // namespace ToolKit
