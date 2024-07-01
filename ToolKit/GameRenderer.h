/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "ForwardSceneRenderPath.h"
#include "FxaaPass.h"
#include "GammaPass.h"
#include "GammaTonemapFxaaPass.h"
#include "Scene.h"
#include "UIManager.h"
#include "Viewport.h"

namespace ToolKit
{
  struct GameRendererParams
  {
    ViewportPtr viewport = nullptr;
    ScenePtr scene       = nullptr;
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
      m_sceneRenderPath      = nullptr;
      m_uiPass               = nullptr;
      m_gammaTonemapFxaaPass = nullptr;
      m_fullQuadPass         = nullptr;
      m_quadUnlitMaterial    = nullptr;
    }

    void SetParams(const GameRendererParams& gameRendererParams);

    void Render(Renderer* renderer) override;

   private:
    void PreRender(Renderer* renderer) override;
    void PostRender(Renderer* renderer) override;

    GameRendererParams m_params;

    SceneRenderPathPtr m_sceneRenderPath           = nullptr;
    ForwardRenderPassPtr m_uiPass                  = nullptr;
    GammaTonemapFxaaPassPtr m_gammaTonemapFxaaPass = nullptr;
    FullQuadPassPtr m_fullQuadPass                 = nullptr;

    RenderJobArray m_uiRenderJobs;
    RenderData m_uiRenderData;

    MaterialPtr m_quadUnlitMaterial = nullptr;
  };
} // namespace ToolKit
