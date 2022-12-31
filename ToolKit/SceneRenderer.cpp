#pragma once

#include "SceneRenderer.h"

#include "Scene.h"

#include "DebugNew.h"

namespace ToolKit
{

  SceneRenderer::SceneRenderer()
  {
    m_shadowPass         = std::make_shared<ShadowPass>();
    m_forwardRenderPass  = std::make_shared<ForwardRenderPass>();
    m_skyPass            = std::make_shared<CubeMapPass>();
    m_gBufferPass        = std::make_shared<GBufferPass>();
    m_deferredRenderPass = std::make_shared<DeferredRenderPass>();
    m_ssaoPass           = std::make_shared<SSAOPass>();
  }

  SceneRenderer::SceneRenderer(const SceneRenderPassParams& params)
      : SceneRenderer()
  {
    m_params = params;
  }

  SceneRenderer::~SceneRenderer()
  {
    m_shadowPass         = nullptr;
    m_forwardRenderPass  = nullptr;
    m_skyPass            = nullptr;
    m_gBufferPass        = nullptr;
    m_deferredRenderPass = nullptr;
    m_ssaoPass           = nullptr;
  }

  void SceneRenderer::Render(Renderer* renderer)
  {
    PreRender();

    CullDrawList(m_gBufferPass->m_params.entities, m_params.Cam);
    CullDrawList(m_forwardRenderPass->m_params.Entities, m_params.Cam);

    // First stage of the render.
    m_passArray.clear();

    // Gbuffer for deferred render
    m_passArray.push_back(m_gBufferPass);

    // Shadow pass
    m_passArray.push_back(m_shadowPass);

    // SSAO pass
    m_passArray.push_back(m_ssaoPass);

    Technique::Render(renderer);

    // Second stage of the render.
    m_passArray.clear();

    renderer->SetShadowAtlas(
        std::static_pointer_cast<Texture>(m_shadowPass->GetShadowAtlas()));

    // Render non-blended entities with deferred renderer
    m_passArray.push_back(m_deferredRenderPass);

    if (m_drawSky)
    {
      m_passArray.push_back(m_skyPass);
    }

    // Forward render blended entities
    m_passArray.push_back(m_forwardRenderPass);

    Technique::Render(renderer);

    renderer->SetShadowAtlas(nullptr);

    PostRender();
  }

  void SceneRenderer::PreRender()
  {
    SetPassParams();

    m_gBufferPass->InitGBuffers(m_params.MainFramebuffer->GetSettings().width,
                                m_params.MainFramebuffer->GetSettings().height);
  }

  void SceneRenderer::PostRender() {}

  void SceneRenderer::SetPassParams()
  {
    m_shadowPass->m_params.Entities  = m_params.Scene->GetEntities();
    m_shadowPass->m_params.Lights    = m_params.Lights;

    // Give blended entities to forward render, non-blendeds to deferred
    // render

    EntityRawPtrArray opaqueDrawList = m_params.Scene->GetEntities();
    EntityRawPtrArray translucentAndUnlitDrawList;
    m_forwardRenderPass->SeperateTranslucentAndUnlitEntities(
        opaqueDrawList,
        translucentAndUnlitDrawList);

    m_gBufferPass->m_params.entities                = opaqueDrawList;
    m_gBufferPass->m_params.camera                  = m_params.Cam;

    m_deferredRenderPass->m_params.ClearFramebuffer = true;
    m_deferredRenderPass->m_params.GBufferFramebuffer =
        m_gBufferPass->m_framebuffer;

    m_deferredRenderPass->m_params.lights          = m_params.Lights;
    m_deferredRenderPass->m_params.MainFramebuffer = m_params.MainFramebuffer;
    m_deferredRenderPass->m_params.Cam             = m_params.Cam;
    m_deferredRenderPass->m_params.AOTexture       = m_ssaoPass->m_ssaoTexture;

    m_forwardRenderPass->m_params.Lights           = m_params.Lights;
    m_forwardRenderPass->m_params.Cam              = m_params.Cam;
    m_forwardRenderPass->m_params.FrameBuffer      = m_params.MainFramebuffer;
    m_forwardRenderPass->m_params.ClearFrameBuffer = false;

    m_forwardRenderPass->m_params.Entities  = translucentAndUnlitDrawList;

    m_ssaoPass->m_params.GPositionBuffer    = m_gBufferPass->m_gPosRt;
    m_ssaoPass->m_params.GNormalBuffer      = m_gBufferPass->m_gNormalRt;
    m_ssaoPass->m_params.GLinearDepthBuffer = m_gBufferPass->m_gLinearDepthRt;
    m_ssaoPass->m_params.Cam                = m_params.Cam;

    // Set CubeMapPass for sky.
    m_drawSky                               = false;
    if (SkyBase* sky = m_params.Scene->GetSky())
    {
      if (m_drawSky = sky->GetDrawSkyVal())
      {
        m_skyPass->m_params.FrameBuffer = m_params.MainFramebuffer;
        m_skyPass->m_params.Cam         = m_params.Cam;
        m_skyPass->m_params.Transform   = sky->m_node->GetTransform();
        m_skyPass->m_params.Material    = sky->GetSkyboxMaterial();
      }
    }
  }

  void SceneRenderer::CullDrawList(EntityRawPtrArray& entities, Camera* camera)
  {
    // Dropout non visible & drawable entities.
    entities.erase(std::remove_if(entities.begin(),
                                  entities.end(),
                                  [](Entity* ntt) -> bool {
                                    return !ntt->GetVisibleVal() ||
                                           !ntt->IsDrawable();
                                  }),
                   entities.end());

    FrustumCull(entities, camera);
  }

} // namespace ToolKit
