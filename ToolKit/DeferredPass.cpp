/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "DeferredPass.h"

#include "Camera.h"
#include "DataTexture.h"
#include "FullQuadPass.h"
#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "TKProfiler.h"
#include "ToolKit.h"

namespace ToolKit
{

  DeferredRenderPass::DeferredRenderPass()
  {
    m_fullQuadPass         = MakeNewPtr<FullQuadPass>();
    m_deferredRenderShader = GetShaderManager()->Create<Shader>(ShaderPath("deferredRenderFrag.shader", true));
    InitLightDataTexture();

    m_deferredRenderShader->AddShaderUniform(ShaderUniform("camPos", ZERO));
    m_deferredRenderShader->AddShaderUniform(ShaderUniform("lightDataTextureWidth", 0.0f));
    m_deferredRenderShader->AddShaderUniform(ShaderUniform("shadowDirLightsInterval", Vec2()));
    m_deferredRenderShader->AddShaderUniform(ShaderUniform("shadowPointLightsInterval", Vec2()));
    m_deferredRenderShader->AddShaderUniform(ShaderUniform("shadowSpotLightsInterval", Vec2()));
    m_deferredRenderShader->AddShaderUniform(ShaderUniform("nonShadowDirLightsInterval", Vec2()));
    m_deferredRenderShader->AddShaderUniform(ShaderUniform("nonShadowPointLightsInterval", Vec2()));
    m_deferredRenderShader->AddShaderUniform(ShaderUniform("nonShadowSpotLightsInterval", Vec2()));
    m_deferredRenderShader->AddShaderUniform(ShaderUniform("dirShadowLightDataSize", 0.0f));
    m_deferredRenderShader->AddShaderUniform(ShaderUniform("pointShadowLightDataSize", 0.0f));
    m_deferredRenderShader->AddShaderUniform(ShaderUniform("spotShadowLightDataSize", 0.0f));
    m_deferredRenderShader->AddShaderUniform(ShaderUniform("dirNonShadowLightDataSize", 0.0f));
    m_deferredRenderShader->AddShaderUniform(ShaderUniform("pointNonShadowLightDataSize", 0.0f));
    m_deferredRenderShader->AddShaderUniform(ShaderUniform("spotNonShadowLightDataSize", 0.0f));
    m_deferredRenderShader->AddShaderUniform(ShaderUniform("aoEnabled", false));
  }

  DeferredRenderPass::DeferredRenderPass(const DeferredRenderPassParams& params) : DeferredRenderPass()
  {
    m_params = params;
  }

  DeferredRenderPass::~DeferredRenderPass()
  {
    m_fullQuadPass         = nullptr;
    m_deferredRenderShader = nullptr;
    m_lightDataTexture     = nullptr;
  }

  void DeferredRenderPass::PreRender()
  {
    PUSH_GPU_MARKER("DeferredRenderPas::PreRender");
    PUSH_CPU_MARKER("DeferredRenderPas::PreRender");

    Pass::PreRender();
    m_deferredRenderShader->UpdateShaderUniform("camPos", m_params.Cam->Position());

    m_fullQuadPass->m_params.ClearFrameBuffer = m_params.ClearFramebuffer;
    m_fullQuadPass->m_params.FragmentShader   = m_deferredRenderShader;
    m_fullQuadPass->m_params.FrameBuffer      = m_params.MainFramebuffer;

    Renderer* renderer                        = GetRenderer();

    Vec2 sd, nsd, sp, nsp, ss, nss;
    float sizeD, sizeP, sizeS, sizeND, sizeNP, sizeNS;

    // Update light data texture
    m_lightDataTexture
        ->UpdateTextureData(m_params.lights, sd, sp, ss, nsd, nsp, nss, sizeD, sizeP, sizeS, sizeND, sizeNP, sizeNS);

    // Update light uniforms
    m_deferredRenderShader->UpdateShaderUniform("lightDataTextureWidth", (float) m_lightDataTextureSize.x);
    m_deferredRenderShader->UpdateShaderUniform("shadowDirLightsInterval", sd);
    m_deferredRenderShader->UpdateShaderUniform("shadowPointLightsInterval", sp);
    m_deferredRenderShader->UpdateShaderUniform("nonShadowDirLightsInterval", nsd);
    m_deferredRenderShader->UpdateShaderUniform("nonShadowPointLightsInterval", nsp);
    m_deferredRenderShader->UpdateShaderUniform("nonShadowSpotLightsInterval", nss);
    m_deferredRenderShader->UpdateShaderUniform("dirShadowLightDataSize", sizeD);
    m_deferredRenderShader->UpdateShaderUniform("pointShadowLightDataSize", sizeP);
    m_deferredRenderShader->UpdateShaderUniform("spotShadowLightDataSize", sizeS);
    m_deferredRenderShader->UpdateShaderUniform("dirNonShadowLightDataSize", sizeND);
    m_deferredRenderShader->UpdateShaderUniform("pointNonShadowLightDataSize", sizeNP);
    m_deferredRenderShader->UpdateShaderUniform("shadowSpotLightsInterval", ss);
    m_deferredRenderShader->UpdateShaderUniform("spotNonShadowLightDataSize", sizeNS);

    // Set gbuffer
    // 9: Position, 10: Normal, 11: Color, 12: emissive, 14: metallic-roughness,
    // 16: ibl contribution
    using FAttachment           = Framebuffer::Attachment;
    FramebufferPtr gFrameBuffer = m_params.GBufferFramebuffer;

    renderer->SetTexture(9, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment0)->m_textureId);
    renderer->SetTexture(10, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment1)->m_textureId);
    renderer->SetTexture(11, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment2)->m_textureId);
    renderer->SetTexture(12, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment3)->m_textureId);
    renderer->SetTexture(14, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment5)->m_textureId);
    renderer->SetTexture(16, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment6)->m_textureId);

    renderer->SetTexture(13, m_lightDataTexture->m_textureId);

    m_deferredRenderShader->UpdateShaderUniform("aoEnabled", m_params.AOTexture != nullptr);
    renderer->SetTexture(5, m_params.AOTexture ? m_params.AOTexture->m_textureId : 0);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void DeferredRenderPass::PostRender()
  {
    PUSH_GPU_MARKER("DeferredRenderPass::PostRender");
    PUSH_CPU_MARKER("DeferredRenderPass::PostRender");

    // Copy real depth buffer to main framebuffer depth
    GetRenderer()->CopyFrameBuffer(m_params.GBufferFramebuffer, m_params.MainFramebuffer, GraphicBitFields::DepthBits);
    Pass::PostRender();

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void DeferredRenderPass::Render()
  {
    PUSH_GPU_MARKER("DeferredRenderPass::Render");
    PUSH_CPU_MARKER("DeferredRenderPass::Render");

    // Deferred render always uses PBR material
    RenderSubPass(m_fullQuadPass);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void DeferredRenderPass::InitLightDataTexture()
  {
    m_lightDataTexture = MakeNewPtr<LightDataTexture>(m_lightDataTextureSize.x, m_lightDataTextureSize.y);
    m_lightDataTexture->Init();
  }

} // namespace ToolKit