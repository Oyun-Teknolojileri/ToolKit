/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "GammaTonemapFxaaPass.h"

#include "TKProfiler.h"

namespace ToolKit
{

  GammaTonemapFxaaPass::GammaTonemapFxaaPass() : PostProcessPass()
  {
    m_postProcessShader = GetShaderManager()->Create<Shader>(ShaderPath("gammaTonemapFxaa.shader", true));
  }

  GammaTonemapFxaaPass::GammaTonemapFxaaPass(const GammaTonemapFxaaPassParams& params) : GammaTonemapFxaaPass()
  {
    m_params = params;
  }

  void GammaTonemapFxaaPass::PreRender()
  {
    PUSH_GPU_MARKER("FXAAPass::PreRender");
    PUSH_CPU_MARKER("FXAAPass::PreRender");

    PostProcessPass::m_params.FrameBuffer = m_params.frameBuffer;
    PostProcessPass::PreRender();

    m_postProcessPass->UpdateUniform(ShaderUniform("enableFxaa", (int) m_params.enableFxaa));
    m_postProcessPass->UpdateUniform(ShaderUniform("enableGammaCorrection", (int) m_params.enableGammaCorrection));
    m_postProcessPass->UpdateUniform(ShaderUniform("enableTonemapping", (int) m_params.enableTonemapping));

    m_postProcessPass->UpdateUniform(ShaderUniform("screenSize", m_params.screenSize));
    m_postProcessPass->UpdateUniform(ShaderUniform("useAcesTonemapper", (uint) m_params.tonemapMethod));
    m_postProcessPass->UpdateUniform(ShaderUniform("gamma", m_params.gamma));

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

} // namespace ToolKit