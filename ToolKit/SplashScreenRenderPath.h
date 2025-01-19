#pragma once

#include "ForwardPass.h"
#include "GammaTonemapFxaaPass.h"
#include "RenderSystem.h"
#include "UIManager.h"
#include "Viewport.h"

namespace ToolKit
{

  class TK_API SpashScreenRenderPath : public RenderPath
  {
   public:
    SpashScreenRenderPath() {};
    virtual ~SpashScreenRenderPath() {};

    /** Initialize all resources that will be used to render splash screen. */
    void Init()
    {
      m_uiPass       = MakeNewPtr<ForwardRenderPass>();
      m_gammaPass    = MakeNewPtr<GammaTonemapFxaaPass>();
      m_viewport     = MakeNewPtr<Viewport>();
      m_splashScreen = MakeNewPtr<UILayer>(LayerPath("ToolKit/splash-screen.layer"));
    }

   private:
    ForwardRenderPassPtr m_uiPass       = nullptr;
    GammaTonemapFxaaPassPtr m_gammaPass = nullptr;
    ViewportPtr m_viewport              = nullptr;
    UILayerPtr m_splashScreen           = nullptr;
  };

} // namespace ToolKit