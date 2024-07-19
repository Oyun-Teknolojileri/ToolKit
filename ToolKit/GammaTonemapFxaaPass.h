/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "PostProcessPass.h"

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
    GammaTonemapFxaaPass(const GammaTonemapFxaaPassParams& params);

    void PreRender();

    /** Returns true if any of the sub passes (Tonemap, Fxaa, Gamma) are required. */
    bool IsEnabled();

   public:
    GammaTonemapFxaaPassParams m_params;
  };

} // namespace ToolKit