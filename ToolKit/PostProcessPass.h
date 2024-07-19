/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "FullQuadPass.h"
#include "Pass.h"

namespace ToolKit
{

  enum class DoFQuality
  {
    Low,    // Radius Scale = 2.0f
    Normal, // Radius Scale = 0.8f
    High    // Radius Scale = 0.2f
  };

  enum class TonemapMethod
  {
    Reinhard,
    Aces
  };

  struct PostProcessPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    ShaderPtr Shader           = nullptr;
  };

  class TK_API PostProcessPass : public Pass
  {
   public:
    PostProcessPass();
    PostProcessPass(const PostProcessPassParams& params);
    ~PostProcessPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    PostProcessPassParams m_params;

   protected:
    ShaderPtr m_postProcessShader     = nullptr;
    FullQuadPassPtr m_postProcessPass = nullptr;
    FramebufferPtr m_copyBuffer       = nullptr;
    RenderTargetPtr m_copyTexture     = nullptr;
  };

} // namespace ToolKit