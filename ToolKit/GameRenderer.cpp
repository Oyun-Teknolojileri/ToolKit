#include "GameRenderer.h"

#include "stdafx.h"

namespace ToolKit
{
  GameRenderer::GameRenderer()
  {
    m_mobileSceneRenderPath = MakeNewPtr<MobileSceneRenderPath>();
    m_sceneRenderPath       = MakeNewPtr<SceneRenderPath>();
    m_uiPass                = MakeNewPtr<ForwardRenderPass>();
    m_tonemapPass           = MakeNewPtr<TonemapPass>();
    m_gammaPass             = MakeNewPtr<GammaPass>();
    m_fxaaPass              = MakeNewPtr<FXAAPass>();
    m_fullQuadPass          = MakeNewPtr<FullQuadPass>();
  }

  void GameRenderer::PreRender()
  {
    if (m_params.useMobileRenderPath)
    {
      // Mobile pass params

      m_mobileSceneRenderPath->m_params.Cam                        = m_params.viewport->GetCamera();
      m_mobileSceneRenderPath->m_params.Lights                     = m_params.scene->GetLights();
      m_mobileSceneRenderPath->m_params.MainFramebuffer            = m_params.viewport->m_framebuffer;
      m_mobileSceneRenderPath->m_params.Scene                      = m_params.scene;
      m_mobileSceneRenderPath->m_params.Gfx                        = m_params.gfx;

      // These post processings will be done after ui pass
      m_mobileSceneRenderPath->m_params.Gfx.GammaCorrectionEnabled = false;
      m_mobileSceneRenderPath->m_params.Gfx.TonemappingEnabled     = false;
      m_mobileSceneRenderPath->m_params.Gfx.FXAAEnabled            = false;
    }
    else
    {
      // Scene render params

      m_sceneRenderPath->m_params.Cam                        = m_params.viewport->GetCamera();
      m_sceneRenderPath->m_params.ClearFramebuffer           = true;
      m_sceneRenderPath->m_params.Lights                     = m_params.scene->GetLights();
      m_sceneRenderPath->m_params.MainFramebuffer            = m_params.viewport->m_framebuffer;
      m_sceneRenderPath->m_params.Scene                      = m_params.scene;
      m_sceneRenderPath->m_params.Gfx                        = m_params.gfx;

      // These post processings will be done after ui pass
      m_sceneRenderPath->m_params.Gfx.GammaCorrectionEnabled = false;
      m_sceneRenderPath->m_params.Gfx.TonemappingEnabled     = false;
      m_sceneRenderPath->m_params.Gfx.FXAAEnabled            = false;
    }

    // UI params

    UILayerPtrArray layers;
    m_uiRenderData.jobs.clear();
    GetUIManager()->GetLayers(m_params.viewport->m_viewportId, layers);

    for (const UILayerPtr& layer : layers)
    {
      const EntityPtrArray& uiNtties = layer->m_scene->GetEntities();
      RenderJobProcessor::CreateRenderJobs(uiNtties, m_uiRenderData.jobs);
    }

    RenderJobProcessor::SeperateRenderData(m_uiRenderData, true);

    m_uiPass->m_params.renderData             = &m_uiRenderData;
    m_uiPass->m_params.Cam                    = GetUIManager()->GetUICamera();
    m_uiPass->m_params.FrameBuffer            = m_params.viewport->m_framebuffer;
    m_uiPass->m_params.clearBuffer            = GraphicBitFields::DepthBits;

    // Tonemap Pass

    m_tonemapPass->m_params.FrameBuffer       = m_params.viewport->m_framebuffer;
    m_tonemapPass->m_params.Method            = m_params.gfx.TonemapperMode;

    // Gamma Pass

    m_gammaPass->m_params.FrameBuffer         = m_params.viewport->m_framebuffer;
    m_gammaPass->m_params.Gamma               = m_params.gfx.Gamma;

    // FXAA Pass

    m_fxaaPass->m_params.FrameBuffer          = m_params.viewport->m_framebuffer;
    m_fxaaPass->m_params.screen_size          = m_params.viewport->m_wndContentAreaSize;

    // Full quad pass

    m_fullQuadPass->m_params.FrameBuffer      = nullptr; // backbuffer
    m_fullQuadPass->m_params.ClearFrameBuffer = true;
    if (m_unlitMaterial == nullptr)
    {
      m_unlitMaterial = GetMaterialManager()->GetCopyOfUnlitMaterial(false);
    }
    m_unlitMaterial->m_diffuseTexture =
        Cast<Texture>(m_params.viewport->m_framebuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0));
    m_fullQuadPass->m_material = m_unlitMaterial;
  }

  void GameRenderer::PostRender(Renderer* renderer) { renderer->ResetUsedTextureSlots(); }

  void GameRenderer::SetParams(const GameRendererParams& gameRendererParams) { m_params = gameRendererParams; }

  void GameRenderer::Render(Renderer* renderer)
  {
    if (m_params.scene == nullptr || m_params.viewport == nullptr)
    {
      return;
    }

    PreRender();

    m_passArray.clear();

    // Scene renderer
    SceneRenderPathPtr sceneRenderer = nullptr;
    if (m_params.useMobileRenderPath)
    {
      sceneRenderer = m_mobileSceneRenderPath;
    }
    else
    {
      sceneRenderer = m_sceneRenderPath;
    }
    sceneRenderer->Render(renderer);

    // UI render pass
    m_passArray.push_back(m_uiPass);

    RenderPath::Render(renderer);
    m_passArray.clear();

    // Post processings
    if (m_params.gfx.TonemappingEnabled)
    {
      m_passArray.push_back(m_tonemapPass);
    }

    RenderPath::Render(renderer);
    m_passArray.clear();

    if (m_params.gfx.FXAAEnabled)
    {
      m_passArray.push_back(m_fxaaPass);
    }

    RenderPath::Render(renderer);
    m_passArray.clear();

    if (m_params.gfx.GammaCorrectionEnabled && GetRenderSystem()->IsGammaCorrectionNeeded())
    {
      m_passArray.push_back(m_gammaPass);
    }

    RenderPath::Render(renderer);
    m_passArray.clear();

    m_passArray.push_back(m_fullQuadPass);
    RenderPath::Render(renderer);

    PostRender(renderer);
  }
} // namespace ToolKit
