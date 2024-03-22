#pragma once

#include "PostProcessPass.h"
#include "ToneMapPass.h"

namespace ToolKit
{
  struct GammaTonemapFxaaPassParams
  {
    bool enableGammaCorrection = true;
    bool enableTonemapping     = true;
    bool enableFxaa            = true;

    FramebufferPtr frameBuffer = nullptr;
    Vec2 screenSize;
    TonemapMethod tonemapMethod = TonemapMethod::Aces;
    float gamma                 = 2.2f;
  };

  class TK_API GammaTonemapFxaaPass : public PostProcessPass
  {
   public:
    GammaTonemapFxaaPass();
    explicit GammaTonemapFxaaPass(const GammaTonemapFxaaPassParams& params);

    void PreRender();

   public:
    GammaTonemapFxaaPassParams m_params;
  };
} // namespace ToolKit