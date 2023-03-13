#pragma once

#include "Pass.h"
#include "PostProcessPass.h"
#include "RenderSystem.h"
#include "ToolKit.h"

namespace ToolKit
{

  struct SceneRenderPassParams
  {
    LightRawPtrArray Lights;
    ScenePtr Scene                 = nullptr;
    Camera* Cam                    = nullptr;
    FramebufferPtr MainFramebuffer = nullptr;
    bool ClearFramebuffer          = true;
    EngineSettings::PostProcessingSettings Gfx;
  };

  /**
   * Main scene renderer.
   */
  class TK_API SceneRenderer : public Technique
  {
   public:
    SceneRenderer();
    explicit SceneRenderer(const SceneRenderPassParams& params);
    virtual ~SceneRenderer();

    void Render(Renderer* renderer) override;
    void PreRender(Renderer* renderer);
    void PostRender();

   private:
    void SetPassParams();
    void CullDrawList(EntityRawPtrArray& entities, Camera* camera);

   public:
    SceneRenderPassParams m_params;

   public:
    ShadowPassPtr m_shadowPass                 = nullptr;
    ForwardRenderPassPtr m_forwardRenderPass   = nullptr;
    CubeMapPassPtr m_skyPass                   = nullptr;
    GBufferPassPtr m_gBufferPass               = nullptr;
    DeferredRenderPassPtr m_deferredRenderPass = nullptr;
    SSAOPassPtr m_ssaoPass                     = nullptr;
    FXAAPassPtr m_fxaaPass                     = nullptr;
    GammaPassPtr m_gammaPass                   = nullptr;
    BloomPassPtr m_bloomPass                   = nullptr;
    TonemapPassPtr m_tonemapPass               = nullptr;
    DoFPassPtr m_dofPass                       = nullptr;
    LightRawPtrArray m_updatedLights;

   private:
    bool m_drawSky = false;
    SkyBase* m_sky = nullptr;
  };

  typedef std::shared_ptr<SceneRenderer> SceneRendererPtr;
} // namespace ToolKit