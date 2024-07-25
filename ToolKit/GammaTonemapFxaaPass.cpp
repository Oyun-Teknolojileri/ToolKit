/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "GammaTonemapFxaaPass.h"

#include "Material.h"
#include "TKProfiler.h"

namespace ToolKit
{

  GammaTonemapFxaaPass::GammaTonemapFxaaPass()
  {
    m_quadPass          = MakeNewPtr<FullQuadPass>();
    m_processTexture    = MakeNewPtr<RenderTarget>();
    m_postProcessShader = GetShaderManager()->Create<Shader>(ShaderPath("gammaTonemapFxaa.shader", true));
  }

  GammaTonemapFxaaPass::GammaTonemapFxaaPass(const GammaTonemapFxaaPassParams& params) : GammaTonemapFxaaPass()
  {
    m_params = params;
  }

  void GammaTonemapFxaaPass::PreRender()
  {
    Pass::PreRender();

    RenderTargetPtr srcTexture = m_params.frameBuffer->GetColorAttachment(Framebuffer::Attachment::ColorAttachment0);
    m_quadPass->m_material->m_diffuseTexture = srcTexture;
    m_quadPass->m_material->m_fragmentShader = m_postProcessShader;
    m_quadPass->m_params.frameBuffer         = m_params.frameBuffer;
    m_quadPass->m_params.clearFrameBuffer    = GraphicBitFields::AllBits;

    m_quadPass->UpdateUniform(ShaderUniform("enableFxaa", (int) m_params.enableFxaa));
    m_quadPass->UpdateUniform(ShaderUniform("enableGammaCorrection", (int) m_params.enableGammaCorrection));
    m_quadPass->UpdateUniform(ShaderUniform("enableTonemapping", (int) m_params.enableTonemapping));

    m_quadPass->UpdateUniform(ShaderUniform("screenSize", m_params.screenSize));
    m_quadPass->UpdateUniform(ShaderUniform("useAcesTonemapper", (uint) m_params.tonemapMethod));
    m_quadPass->UpdateUniform(ShaderUniform("gamma", m_params.gamma));

    m_processTexture->ReconstructIfNeeded(srcTexture->m_width, srcTexture->m_height, &srcTexture->Settings());
    m_params.frameBuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_processTexture);

    Renderer* renderer = GetRenderer();
    // renderer->ClearBuffer(GraphicBitFields::AllBits);
  }

  void GammaTonemapFxaaPass::Render() { RenderSubPass(m_quadPass); }

  bool GammaTonemapFxaaPass::IsEnabled()
  {
    return m_params.enableFxaa || m_params.enableGammaCorrection || m_params.enableTonemapping;
  }

} // namespace ToolKit