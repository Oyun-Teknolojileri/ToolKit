#pragma once

#include "FxaaPass.h"
#include "GammaPass.h"
#include "GammaTonemapFxaaPass.h"
#include "MobileSceneRenderPath.h"
#include "Scene.h"
#include "SceneRenderPath.h"
#include "UIManager.h"
#include "Viewport.h"

namespace ToolKit
{
  struct GameRendererParams
  {
    Viewport* viewport       = nullptr;
    ScenePtr scene           = nullptr;
    bool useMobileRenderPath = false;
    EngineSettings::PostProcessingSettings gfx;
  };

  /**
   * This class is being used in games to render.
   */
  class TK_API GameRenderer : public RenderPath
  {
   public:
    GameRenderer();

    virtual ~GameRenderer()
    {
      m_sceneRenderPath       = nullptr;
      m_mobileSceneRenderPath = nullptr;
      m_uiPass                = nullptr;
      m_gammaTonemapFxaaPass  = nullptr;
      m_fullQuadPass          = nullptr;

      m_unlitMaterial         = nullptr;
    }

    void SetParams(const GameRendererParams& gameRendererParams);

    void Render(Renderer* renderer) override;

   private:
    void PreRender(Renderer* renderer);
    void PostRender(Renderer* renderer);

    GameRendererParams m_params;

    SceneRenderPathPtr m_sceneRenderPath             = nullptr;
    MobileSceneRenderPathPtr m_mobileSceneRenderPath = nullptr;
    ForwardRenderPassPtr m_uiPass                    = nullptr;
    GammaTonemapFxaaPassPtr m_gammaTonemapFxaaPass   = nullptr;
    FullQuadPassPtr m_fullQuadPass                   = nullptr;

    RenderJobArray m_uiRenderJobs;
    RenderData m_uiRenderData;

    MaterialPtr m_unlitMaterial = nullptr;
  };
} // namespace ToolKit
