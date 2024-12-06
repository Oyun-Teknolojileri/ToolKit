/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "ForwardPass.h"
#include "Material.h"
#include "Pass.h"
#include "Texture.h"

namespace ToolKit
{
  /**
   * This pass creates a frame buffer that contains normal of the scene, depth in the view space.
   * If this pass exist, the depth of the scene is constructed and upcoming passes uses the existing
   * depth buffer without write to enable early z kill in the fragment shader.
   **/
  class TK_API ForwardPreProcessPass : public Pass
  {
   public:
    ForwardPreProcessPass();

    void InitBuffers(int width, int height, int sampleCount);
    void Render() override;
    void PreRender() override;

   public:
    ForwardRenderPassParams m_params;
    MaterialPtr m_linearMaterial    = nullptr;
    FramebufferPtr m_framebuffer    = nullptr;

    RenderTargetPtr m_normalRt      = nullptr;
    RenderTargetPtr m_linearDepthRt = nullptr;
  };

  typedef std::shared_ptr<ForwardPreProcessPass> ForwardPreProcessPassPtr;

} // namespace ToolKit
