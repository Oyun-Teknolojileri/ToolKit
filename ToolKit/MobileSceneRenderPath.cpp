/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "MobileSceneRenderPath.h"

#include "Material.h"
#include "Scene.h"
#include "Shader.h"

namespace ToolKit
{
  MobileSceneRenderPath::MobileSceneRenderPath()
  {
    m_shadowPass            = MakeNewPtr<ShadowPass>();
    m_forwardRenderPass     = MakeNewPtr<ForwardRenderPass>();
    m_skyPass               = MakeNewPtr<CubeMapPass>();
    m_forwardPreProcessPass = MakeNewPtr<ForwardPreProcess>();
    m_ssaoPass              = MakeNewPtr<SSAOPass>();
    m_fxaaPass              = MakeNewPtr<FXAAPass>();
    m_bloomPass             = MakeNewPtr<BloomPass>();
    m_tonemapPass           = MakeNewPtr<TonemapPass>();
    m_dofPass               = MakeNewPtr<DoFPass>();
  }

  MobileSceneRenderPath::MobileSceneRenderPath(const SceneRenderPathParams& params) { m_params = params; }

  MobileSceneRenderPath::~MobileSceneRenderPath()
  {
    m_shadowPass            = nullptr;
    m_forwardRenderPass     = nullptr;
    m_skyPass               = nullptr;
    m_ssaoPass              = nullptr;
    m_forwardPreProcessPass = nullptr;
    m_fxaaPass              = nullptr;
    m_bloomPass             = nullptr;
    m_tonemapPass           = nullptr;
    m_dofPass               = nullptr;
  }

  void MobileSceneRenderPath::Render(Renderer* renderer)
  {
    PreRender(renderer);

    m_passArray.clear();

    renderer->m_sky = m_sky;

    renderer->SetShadowAtlas(Cast<Texture>(m_shadowPass->GetShadowAtlas()));

    // Shadow pass
    m_passArray.push_back(m_shadowPass);

    // Draw sky pass
    if (m_drawSky)
    {
      m_passArray.push_back(m_skyPass);
    }

    // Forward Pre Process Pass
    m_passArray.push_back(m_forwardPreProcessPass);

    // SSAO pass
    if (m_params.Gfx.SSAOEnabled)
    {
      m_passArray.push_back(m_ssaoPass);
    }

    // Forward pass
    m_passArray.push_back(m_forwardRenderPass);

    // Bloom pass
    if (m_params.Gfx.BloomEnabled)
    {
      m_passArray.push_back(m_bloomPass);
    }

    // Bloom pass
    if (m_params.Gfx.TonemappingEnabled)
    {
      m_passArray.push_back(m_tonemapPass);
    }

    // Depth of field pass
    if (m_params.Gfx.DepthOfFieldEnabled)
    {
      m_passArray.push_back(m_dofPass);
    }

    // Fxaa pass
    if (m_params.Gfx.FXAAEnabled)
    {
      m_passArray.push_back(m_fxaaPass);
    }

    RenderPath::Render(renderer);

    renderer->SetShadowAtlas(nullptr);

    PostRender();
  }

  void MobileSceneRenderPath::PreRender(Renderer* renderer)
  {
    renderer->ResetTextureSlots();

    SetPassParams();

    m_forwardPreProcessPass->InitBuffers(m_params.MainFramebuffer->GetSettings().width,
                                         m_params.MainFramebuffer->GetSettings().height);
  }

  void MobileSceneRenderPath::PostRender() { m_updatedLights.clear(); }

  void MobileSceneRenderPath::SetPassParams()
  {
    // Update all lights before using them.
    m_updatedLights = m_params.Lights.empty() ? m_params.Scene->GetLights() : m_params.Lights;

    for (LightPtr light : m_updatedLights)
    {
      light->UpdateShadowCamera();
    }

    const EntityPtrArray& allDrawList = m_params.Scene->GetEntities();

    m_jobs.clear();
    m_params.Scene->m_boundingBox       = RenderJobProcessor::CreateRenderJobs(allDrawList, m_jobs);
    m_shadowPass->m_params.shadowVolume = m_params.Scene->m_boundingBox;

    m_shadowPass->m_params.RendeJobs    = m_jobs;
    m_shadowPass->m_params.Lights       = m_updatedLights;
    m_shadowPass->m_params.ViewCamera   = m_params.Cam;

    RenderJobProcessor::CullRenderJobs(m_jobs, m_params.Cam);

    RenderJobProcessor::AssignEnvironment(m_jobs, m_params.Scene->GetEnvironmentVolumes());

    RenderJobArray opaque, translucent;
    RenderJobProcessor::SeperateOpaqueTranslucent(m_jobs, opaque, translucent);

    // Set all shaders as forward shader
    // Translucent has already forward shader
    ShaderManager* shaderMan = GetShaderManager();
    for (RenderJob& job : opaque)
    {
      if (job.Material->m_fragmentShader->GetFile() == shaderMan->PbrDefferedShaderFile())
      {
        job.Material->m_fragmentShader = shaderMan->GetPbrForwardShader();
      }
    }

    // Set CubeMapPass for sky.
    m_drawSky         = false;
    bool couldDrawSky = false;
    if (m_sky = m_params.Scene->GetSky())
    {
      if (m_drawSky = m_sky->GetDrawSkyVal())
      {
        m_skyPass->m_params.ClearFramebuffer = m_params.ClearFramebuffer;
        m_skyPass->m_params.FrameBuffer      = m_params.MainFramebuffer;
        m_skyPass->m_params.Cam              = m_params.Cam;
        m_skyPass->m_params.Transform        = m_sky->m_node->GetTransform();
        m_skyPass->m_params.Material         = m_sky->GetSkyboxMaterial();

        couldDrawSky                         = true;
      }
    }

    m_forwardRenderPass->m_params.Lights          = m_updatedLights;
    m_forwardRenderPass->m_params.Cam             = m_params.Cam;
    m_forwardRenderPass->m_params.FrameBuffer     = m_params.MainFramebuffer;
    m_forwardRenderPass->m_params.SSAOEnabled     = m_params.Gfx.SSAOEnabled;
    m_forwardRenderPass->m_params.OpaqueJobs      = opaque;
    m_forwardRenderPass->m_params.TranslucentJobs = translucent;
    m_forwardRenderPass->m_params.SsaoTexture     = m_ssaoPass->m_ssaoTexture;

    // If sky is being rendered, then clear the main framebuffer there. If sky pass is not rendered, clear the
    // framebuffer here
    if (!couldDrawSky)
    {
      GraphicBitFields clearBuffer = m_params.ClearFramebuffer ? GraphicBitFields::AllBits : GraphicBitFields::None;
      m_forwardRenderPass->m_params.clearBuffer = clearBuffer;
    }
    else
    {
      m_forwardRenderPass->m_params.clearBuffer = GraphicBitFields::None;
    }

    m_forwardPreProcessPass->m_params       = m_forwardRenderPass->m_params;

    m_ssaoPass->m_params.GNormalBuffer      = m_forwardPreProcessPass->m_normalRt;
    m_ssaoPass->m_params.GLinearDepthBuffer = m_forwardPreProcessPass->m_linearDepthRt;
    m_ssaoPass->m_params.Cam                = m_params.Cam;
    m_ssaoPass->m_params.Radius             = m_params.Gfx.SSAORadius;
    m_ssaoPass->m_params.spread             = m_params.Gfx.SSAOSpread;
    m_ssaoPass->m_params.Bias               = m_params.Gfx.SSAOBias;
    m_ssaoPass->m_params.KernelSize         = m_params.Gfx.SSAOKernelSize;

    m_bloomPass->m_params.FrameBuffer       = m_params.MainFramebuffer;
    m_bloomPass->m_params.intensity         = m_params.Gfx.BloomIntensity;
    m_bloomPass->m_params.minThreshold      = m_params.Gfx.BloomThreshold;
    m_bloomPass->m_params.iterationCount    = m_params.Gfx.BloomIterationCount;

    m_tonemapPass->m_params.FrameBuffer     = m_params.MainFramebuffer;
    m_tonemapPass->m_params.Method          = m_params.Gfx.TonemapperMode;

    m_dofPass->m_params.ColorRt    = m_params.MainFramebuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0);
    m_dofPass->m_params.DepthRt    = m_forwardPreProcessPass->m_linearDepthRt;
    m_dofPass->m_params.focusPoint = m_params.Gfx.FocusPoint;
    m_dofPass->m_params.focusScale = m_params.Gfx.FocusScale;
    m_dofPass->m_params.blurQuality  = m_params.Gfx.DofQuality;

    m_fxaaPass->m_params.FrameBuffer = m_params.MainFramebuffer;
    const FramebufferSettings& fbs   = m_params.MainFramebuffer->GetSettings();
    m_fxaaPass->m_params.screen_size = Vec2(fbs.width, fbs.height);
  }
} // namespace ToolKit