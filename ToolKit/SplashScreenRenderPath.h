#pragma once

#include "ForwardPass.h"
#include "GameViewport.h"
#include "GammaTonemapFxaaPass.h"
#include "RenderSystem.h"
#include "Scene.h"
#include "UIManager.h"

namespace ToolKit
{

  class TK_API SplashScreenRenderPath : public RenderPath
  {
   public:
    SplashScreenRenderPath();
    virtual ~SplashScreenRenderPath();

    /** Initialize all resources that will be used to render splash screen. */
    void Init(UVec2 screenSize);

    void PreRender(Renderer* renderer) override;
    void Render(Renderer* renderer) override;
    void PostRender(Renderer* renderer) override;

   private:
    ForwardRenderPassPtr m_uiPass       = nullptr;
    GammaTonemapFxaaPassPtr m_gammaPass = nullptr;
    GameViewportPtr m_viewport          = nullptr;
    UILayerPtr m_splashScreen           = nullptr;
    RenderData m_uiRenderData;
  };

  typedef std::shared_ptr<SplashScreenRenderPath> SplashScreenRenderPathPtr;

} // namespace ToolKit