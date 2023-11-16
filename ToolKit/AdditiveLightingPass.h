/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "FullQuadPass.h"
#include "Pass.h"
#include "Primative.h"

namespace ToolKit
{
  struct LightingPassParams
  {
    LightPtrArray lights              = {};
    FramebufferPtr MainFramebuffer    = nullptr;
    FramebufferPtr GBufferFramebuffer = nullptr;
    bool ClearFramebuffer             = true;
    CameraPtr Cam                     = nullptr;
    TexturePtr AOTexture              = nullptr;
  };

  class TK_API AdditiveLightingPass : public RenderPass
  {
   public:
    AdditiveLightingPass();
    ~AdditiveLightingPass();

    void Init(const LightingPassParams& params);
    void PreRender() override;
    void PostRender() override;
    void Render() override;

   private:
    void SetLightUniforms(LightPtr light, int lightType);

   public:
    LightingPassParams m_params;
    FullQuadPassPtr m_fullQuadPass       = nullptr;
    RenderTargetPtr m_lightingRt         = nullptr;
    FramebufferPtr m_lightingFrameBuffer = nullptr;
    SpherePtr m_sphereEntity             = nullptr;
    SpherePtr m_sphereMesh               = nullptr;
    MaterialPtr m_meshMaterial           = nullptr;
    ShaderPtr m_mergeShader              = nullptr;
    ShaderPtr m_lightingShader           = nullptr;
  };

  typedef std::shared_ptr<AdditiveLightingPass> LightingPassPtr;
} // namespace ToolKit