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

    renderer->SetFramebuffer(m_params.MainFramebuffer, GraphicBitFields::AllBits);

    // Forward pass
    m_passArray.push_back(m_forwardRenderPass);

    RenderPath::Render(renderer);

    PostRender(renderer);
  }

  void ForwardSceneRenderPath::PreRender(Renderer* renderer)
  {
    RenderPath::PreRender(renderer);

    SetPassParams();

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

    RenderJobProcessor::SeperateRenderData(m_renderData, true);
    RenderJobProcessor::SortByMaterial(m_renderData);

    m_forwardRenderPass->m_params.renderData  = &m_renderData;
    m_forwardRenderPass->m_params.Lights      = m_params.Lights;
    m_forwardRenderPass->m_params.Cam         = m_params.Cam;
    m_forwardRenderPass->m_params.FrameBuffer = m_params.MainFramebuffer;
    m_forwardRenderPass->m_params.SSAOEnabled = m_params.Gfx.SSAOEnabled;
    m_forwardRenderPass->m_params.SsaoTexture = m_ssaoPass->m_ssaoTexture;
  }

  bool ForwardSceneRenderPath::RequiresForwardPreProcessPass()
  {
    return m_params.Gfx.SSAOEnabled || m_params.Gfx.DepthOfFieldEnabled;
  }

} // namespace ToolKit