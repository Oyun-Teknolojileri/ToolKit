﻿/*
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

  class TK_API ForwardPreProcess : public RenderPass
  {
   public:
    ForwardPreProcess();
    ~ForwardPreProcess();

    void InitBuffers(int width, int height);
    void Render() override;
    void PreRender() override;
    void PostRender() override;

   private:
    void InitDefaultDepthTexture(int width, int height);

   public:
    ForwardRenderPassParams m_params;
    MaterialPtr m_linearMaterial    = nullptr;
    MaterialPtr m_linearAlphaMaskMaterial    = nullptr;
    FramebufferPtr m_framebuffer    = nullptr;

    DepthTexturePtr m_depthTexture  = nullptr; // This is used in case there is no gbuffer
    RenderTargetPtr m_normalRt      = nullptr;
    RenderTargetPtr m_linearDepthRt = nullptr;
  };

  typedef std::shared_ptr<ForwardPreProcess> ForwardPreProcessPassPtr;

} // namespace ToolKit
