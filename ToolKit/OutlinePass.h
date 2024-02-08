/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Pass.h"
#include "StencilPass.h"

namespace ToolKit
{

  struct OutlinePassParams
  {
    RenderJobArray* RenderJobs = nullptr;
    FramebufferPtr FrameBuffer = nullptr;
    CameraPtr Camera           = nullptr;
    Vec4 OutlineColor          = Vec4(1.0f);
  };

  /**
   * Draws given entities' outlines to the FrameBuffer.
   * TODO: It should be RenderPath instead of Pass
   */
  class TK_API OutlinePass : public Pass
  {
   public:
    OutlinePass();
    explicit OutlinePass(const OutlinePassParams& params);
    ~OutlinePass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    OutlinePassParams m_params;

   private:
    StencilRenderPassPtr m_stencilPass = nullptr;
    FullQuadPassPtr m_outlinePass      = nullptr;
    ShaderPtr m_dilateShader           = nullptr;
    RenderTargetPtr m_stencilAsRt      = nullptr;
  };

  typedef std::shared_ptr<OutlinePass> OutlinePassPtr;

} // namespace ToolKit