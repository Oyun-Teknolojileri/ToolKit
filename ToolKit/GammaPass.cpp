/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "GammaPass.h"

#include "Shader.h"
#include "TKProfiler.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  GammaPass::GammaPass() : PostProcessPass()
  {
    m_postProcessShader = GetShaderManager()->Create<Shader>(ShaderPath("gammaFrag.shader", true));
  }

  GammaPass::GammaPass(const GammaPassParams& params) : GammaPass() { m_params = params; }

  GammaPass::~GammaPass() {}

  void GammaPass::PreRender()
  {
    PUSH_GPU_MARKER("GammaPass::PreRender");
    PUSH_CPU_MARKER("GammaPass::PreRender");

    PostProcessPass::m_params.FrameBuffer = m_params.FrameBuffer;
    PostProcessPass::PreRender();

    m_postProcessShader->SetShaderParameter("Gamma", ParameterVariant(m_params.Gamma));

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

} // namespace ToolKit