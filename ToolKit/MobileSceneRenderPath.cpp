#include "MobileSceneRenderPath.h"

#include "Scene.h"

namespace ToolKit
{
  MobileSceneRenderPath::MobileSceneRenderPath()
  {
    m_shadowPass        = MakeNewPtr<ShadowPass>();
    m_forwardRenderPass = MakeNewPtr<ForwardRenderPass>();
  }

  MobileSceneRenderPath::MobileSceneRenderPath(const MobileSceneRenderPathParams& params) : MobileSceneRenderPath()
  {
    m_params = params;
  }

  MobileSceneRenderPath::~MobileSceneRenderPath()
  {
    m_shadowPass        = nullptr;
    m_forwardRenderPass = nullptr;
    m_skyPass           = nullptr;
  }

  void MobileSceneRenderPath::Render(Renderer* renderer)
  {
    PreRender(renderer);

    m_passArray.clear();


    // Shadow pass
    m_passArray.push_back(m_shadowPass);

    renderer->SetShadowAtlas(std::static_pointer_cast<Texture>(m_shadowPass->GetShadowAtlas()));

    renderer->m_sky = m_sky;
    if (m_drawSky)
    {
      m_passArray.push_back(m_skyPass);
    }

    m_passArray.push_back(m_forwardRenderPass);

    RenderPath::Render(renderer);

    renderer->SetShadowAtlas(nullptr);

    PostRender();
  }

  void ToolKit::MobileSceneRenderPath::PreRender(Renderer* renderer) { SetPassParams(); }

  void MobileSceneRenderPath::PostRender() { m_updatedLights.clear(); }

  void MobileSceneRenderPath::SetPassParams()
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

    RenderJobArray opaque, translucent;
    RenderJobProcessor::SeperateOpaqueTranslucent(jobs, opaque, translucent);

    m_forwardRenderPass->m_params.Lights           = m_updatedLights;
    m_forwardRenderPass->m_params.Cam              = m_params.Cam;
    m_forwardRenderPass->m_params.FrameBuffer      = m_params.MainFramebuffer;
    m_forwardRenderPass->m_params.SSAOEnabled      = m_params.Gfx.SSAOEnabled;
    m_forwardRenderPass->m_params.ClearFrameBuffer = false;
    m_forwardRenderPass->m_params.OpaqueJobs       = opaque;
    m_forwardRenderPass->m_params.TranslucentJobs  = translucent;

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
  }
} // namespace ToolKit