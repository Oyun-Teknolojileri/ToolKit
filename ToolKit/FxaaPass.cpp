/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "FxaaPass.h"

#include "Shader.h"
#include "ShaderReflectionCache.h"
#include "TKProfiler.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  FXAAPass::FXAAPass() : PostProcessPass()
  {
    m_postProcessShader = GetShaderManager()->Create<Shader>(ShaderPath("fxaaFilterFrag.shader", true));
  }

  FXAAPass::FXAAPass(const FXAAPassParams& params) : FXAAPass() { m_params = params; }

  void FXAAPass::PreRender()
  {
    PUSH_GPU_MARKER("FXAAPass::PreRender");
    PUSH_CPU_MARKER("FXAAPass::PreRender");

    PostProcessPass::m_params.FrameBuffer = m_params.FrameBuffer;
    PostProcessPass::PreRender();

    m_postProcessShader->SetShaderParameter("screen_size", ParameterVariant(m_params.screen_size));

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

} // namespace ToolKit