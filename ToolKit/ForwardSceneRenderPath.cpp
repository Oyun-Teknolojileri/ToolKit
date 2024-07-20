/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "ForwardSceneRenderPath.h"

#include "Material.h"
#include "Scene.h"
#include "Shader.h"

namespace ToolKit
{

  ForwardSceneRenderPath::ForwardSceneRenderPath()
  {
    m_shadowPass            = MakeNewPtr<ShadowPass>();
    m_forwardRenderPass     = MakeNewPtr<ForwardRenderPass>();
    m_skyPass               = MakeNewPtr<CubeMapPass>();
    m_forwardPreProcessPass = MakeNewPtr<ForwardPreProcess>();
    m_ssaoPass              = MakeNewPtr<SSAOPass>();
    m_bloomPass             = MakeNewPtr<BloomPass>();
    m_dofPass               = MakeNewPtr<DoFPass>();
  }

  ForwardSceneRenderPath::ForwardSceneRenderPath(const SceneRenderPathParams& params) { m_params = params; }

  ForwardSceneRenderPath::~ForwardSceneRenderPath()
  {
    m_shadowPass            = nullptr;
    m_forwardRenderPass     = nullptr;
    m_skyPass               = nullptr;
    m_ssaoPass              = nullptr;
    m_forwardPreProcessPass = nullptr;
    m_bloomPass             = nullptr;
    m_dofPass               = nullptr;
  }

  void ForwardSceneRenderPath::Render(Renderer* renderer)
  {
    PreRender(renderer);

    m_passArray.clear();

    // Shadow pass
    renderer->SetShadowAtlas(Cast<Texture>(m_shadowPass->GetShadowAtlas()));
    m_passArray.push_back(m_shadowPass);

    // Forward Pre Process Pass
    if (RequiresForwardPreProcessPass())
    {
      m_passArray.push_back(m_forwardPreProcessPass);
    }

    // SSAO pass
    if (m_params.Gfx.SSAOEnabled)
    {
      m_passArray.push_back(m_ssaoPass);
    }

    // Draw sky pass
    renderer->m_sky = m_sky;
    if (m_drawSky)
    {
      m_passArray.push_back(m_skyPass);
    }

    // Forward pass
    m_passArray.push_back(m_forwardRenderPass);

    // Bloom pass
    if (m_params.Gfx.BloomEnabled)
    {
      m_passArray.push_back(m_bloomPass);
    }

    // Depth of field pass
    if (m_params.Gfx.DepthOfFieldEnabled)
    {
      m_passArray.push_back(m_dofPass);
    }

    RenderPath::Render(renderer);

    renderer->SetShadowAtlas(nullptr);

    PostRender(renderer);
  }

  void ForwardSceneRenderPath::PreRender(Renderer* renderer)
  {
    RenderPath::PreRender(renderer);

    SetPassParams();

    // Init / ReInit forward pre process.
    if (RequiresForwardPreProcessPass())
    {
      FramebufferSettings settings = m_params.MainFramebuffer->GetSettings();
      m_forwardPreProcessPass->InitBuffers(settings.width, settings.height);
    }
  }

  void ForwardSceneRenderPath::PostRender(Renderer* renderer) { RenderPath::PostRender(renderer); }

  void ForwardSceneRenderPath::SetPassParams()
  {
    RenderJobProcessor::CreateRenderJobs(m_renderData.jobs,
                                         m_params.Scene->m_bvh,
                                         m_params.Lights,
                                         m_params.Cam,
                                         m_params.Scene->GetEnvironmentVolumes());

    m_shadowPass->m_params.scene      = m_params.Scene;
    m_shadowPass->m_params.viewCamera = m_params.Cam;
    m_shadowPass->m_params.lights     = m_params.Lights;

    RenderJobProcessor::SeperateRenderData(m_renderData, true);
    RenderJobProcessor::SortByMaterial(m_renderData);

    // Set CubeMapPass for sky.
    m_drawSky         = false;
    bool couldDrawSky = false;
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

    m_forwardRenderPass->m_params.renderData  = &m_renderData;
    m_forwardRenderPass->m_params.Lights      = m_params.Lights;
    m_forwardRenderPass->m_params.Cam         = m_params.Cam;
    m_forwardRenderPass->m_params.FrameBuffer = m_params.MainFramebuffer;
    m_forwardRenderPass->m_params.SSAOEnabled = m_params.Gfx.SSAOEnabled;
    m_forwardRenderPass->m_params.SsaoTexture = m_ssaoPass->m_ssaoTexture;

    // Either sky pass or forward pass will clear the main frame buffer.
    if (m_drawSky)
    {
      m_forwardRenderPass->m_params.clearBuffer = GraphicBitFields::None;
    }
    else
    {
      m_forwardRenderPass->m_params.clearBuffer = GraphicBitFields::AllBits;
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

    m_dofPass->m_params.ColorRt    = m_params.MainFramebuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0);
    m_dofPass->m_params.DepthRt    = m_forwardPreProcessPass->m_linearDepthRt;
    m_dofPass->m_params.focusPoint = m_params.Gfx.FocusPoint;
    m_dofPass->m_params.focusScale = m_params.Gfx.FocusScale;
    m_dofPass->m_params.blurQuality = m_params.Gfx.DofQuality;
  }

  bool ForwardSceneRenderPath::RequiresForwardPreProcessPass()
  {
    return m_params.Gfx.SSAOEnabled || m_params.Gfx.DepthOfFieldEnabled;
  }

} // namespace ToolKit