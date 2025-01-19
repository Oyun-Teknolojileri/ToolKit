#pragma once

#include "ForwardPass.h"
#include "GammaTonemapFxaaPass.h"
#include "RenderSystem.h"
#include "Scene.h"
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
    void Init(UVec2 screenSize)
    {
      m_uiPass       = MakeNewPtr<ForwardRenderPass>();
      m_gammaPass    = MakeNewPtr<GammaTonemapFxaaPass>();
      m_viewport     = MakeNewPtr<Viewport>((float) screenSize.x, (float) screenSize.y);
      m_splashScreen = MakeNewPtr<UILayer>(LayerPath("ToolKit/splash-screen.layer"));

      if (UIManager* uiMan = GetUIManager())
      {
        uiMan->RegisterViewport(m_viewport);
        uiMan->AddLayer(m_viewport->m_viewportId, m_splashScreen);
        m_uiPass->m_params.Cam = uiMan->GetUICamera();
      }

      m_uiPass->m_params.FrameBuffer = m_viewport->m_framebuffer;
    }

    void PreRender(Renderer* renderer) override
    {
      EntityRawPtrArray rawEntities = ToEntityRawPtrArray(m_splashScreen->m_scene->GetEntities());
      RenderJobProcessor::CreateRenderJobs(m_uiRenderData.jobs, rawEntities);
      RenderJobProcessor::SeperateRenderData(m_uiRenderData, true);
    }

   private:
    ForwardRenderPassPtr m_uiPass       = nullptr;
    GammaTonemapFxaaPassPtr m_gammaPass = nullptr;
    ViewportPtr m_viewport              = nullptr;
    UILayerPtr m_splashScreen           = nullptr;
    RenderData m_uiRenderData;
  };

} // namespace ToolKit