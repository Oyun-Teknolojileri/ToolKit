/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "ToneMapPass.h"

#include "Shader.h"
#include "ShaderReflectionCache.h"
#include "TKProfiler.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  TonemapPass::TonemapPass() : PostProcessPass()
  {
    m_postProcessShader = GetShaderManager()->Create<Shader>(ShaderPath("tonemapFrag.shader", true));
  }

  TonemapPass::TonemapPass(const TonemapPassParams& params) : TonemapPass() { m_params = params; }

  TonemapPass::~TonemapPass() {}

  void TonemapPass::PreRender()
  {
    PUSH_GPU_MARKER("TonemapPass::PreRender");
    PUSH_CPU_MARKER("TonemapPass::PreRender");

    PostProcessPass::m_params.FrameBuffer = m_params.FrameBuffer;
    PostProcessPass::PreRender();
    m_postProcessShader->SetShaderParameter("UseAcesTonemapper", ParameterVariant((uint) m_params.Method));

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

} // namespace ToolKit