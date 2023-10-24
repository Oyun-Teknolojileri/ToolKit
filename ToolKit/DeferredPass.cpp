/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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

  DeferredRenderPass::DeferredRenderPass() { m_fullQuadPass = MakeNewPtr<FullQuadPass>(); }

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

    if (m_lightDataTexture == nullptr)
    {
      InitLightDataTexture();
    }

    if (m_deferredRenderShader == nullptr)
    {
      m_deferredRenderShader = GetShaderManager()->Create<Shader>(ShaderPath("deferredRenderFrag.shader", true));
    }
    m_deferredRenderShader->SetShaderParameter("camPos", ParameterVariant(m_params.Cam->m_node->GetTranslation()));

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
    m_deferredRenderShader->SetShaderParameter("lightDataTextureWidth",
                                               ParameterVariant((float) m_lightDataTextureSize.x));
    m_deferredRenderShader->SetShaderParameter("shadowDirLightsInterval", ParameterVariant(sd));
    m_deferredRenderShader->SetShaderParameter("shadowPointLightsInterval", ParameterVariant(sp));
    m_deferredRenderShader->SetShaderParameter("shadowSpotLightsInterval", ParameterVariant(ss));
    m_deferredRenderShader->SetShaderParameter("nonShadowDirLightsInterval", ParameterVariant(nsd));
    m_deferredRenderShader->SetShaderParameter("nonShadowPointLightsInterval", ParameterVariant(nsp));
    m_deferredRenderShader->SetShaderParameter("nonShadowSpotLightsInterval", ParameterVariant(nss));
    m_deferredRenderShader->SetShaderParameter("dirShadowLightDataSize", ParameterVariant(sizeD));
    m_deferredRenderShader->SetShaderParameter("pointShadowLightDataSize", ParameterVariant(sizeP));
    m_deferredRenderShader->SetShaderParameter("spotShadowLightDataSize", ParameterVariant(sizeS));
    m_deferredRenderShader->SetShaderParameter("dirNonShadowLightDataSize", ParameterVariant(sizeND));
    m_deferredRenderShader->SetShaderParameter("pointNonShadowLightDataSize", ParameterVariant(sizeNP));
    m_deferredRenderShader->SetShaderParameter("spotNonShadowLightDataSize", ParameterVariant(sizeNS));

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

    m_deferredRenderShader->SetShaderParameter("aoEnabled", ParameterVariant(m_params.AOTexture != nullptr));
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