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

  struct DoFPassParams
  {
    RenderTargetPtr ColorRt = nullptr;
    RenderTargetPtr DepthRt = nullptr;

    float focusPoint        = 0.0f;
    float focusScale        = 0.0f;
    DoFQuality blurQuality  = DoFQuality::Normal;
  };

  class TK_API DoFPass : public Pass
  {
   public:
    DoFPass();
    explicit DoFPass(const DoFPassParams& params);
    ~DoFPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    DoFPassParams m_params;

   private:
    FullQuadPassPtr m_quadPass    = nullptr;
    ShaderPtr m_dofShader         = nullptr;
    RenderTargetPtr m_copyTexture = nullptr;
  };

  typedef std::shared_ptr<DoFPass> DoFPassPtr;

} // namespace ToolKit