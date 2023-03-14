#include "FxaaPass.h"

#include "Shader.h"
#include "ShaderReflectionCache.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  FXAAPass::FXAAPass() : PostProcessPass()
  {
    m_postProcessShader = GetShaderManager()->Create<Shader>(
        ShaderPath("fxaaFilterFrag.shader", true));
  }

  FXAAPass::FXAAPass(const FXAAPassParams& params) : FXAAPass()
  {
    m_params = params;
  }

  void FXAAPass::PreRender()
  {
    PostProcessPass::m_params.FrameBuffer = m_params.FrameBuffer;
    PostProcessPass::PreRender();

    m_postProcessShader->SetShaderParameter(
        "screen_size",
        ParameterVariant(m_params.screen_size));
  }

} // namespace ToolKit