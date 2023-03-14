#include "ToneMapPass.h"

#include "Shader.h"
#include "ShaderReflectionCache.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  TonemapPass::TonemapPass() : PostProcessPass()
  {
    m_postProcessShader = GetShaderManager()->Create<Shader>(
        ShaderPath("tonemapFrag.shader", true));
  }

  TonemapPass::TonemapPass(const TonemapPassParams& params) : TonemapPass()
  {
    m_params = params;
  }

  TonemapPass::~TonemapPass() {}

  void TonemapPass::PreRender()
  {
    PostProcessPass::m_params.FrameBuffer = m_params.FrameBuffer;
    PostProcessPass::PreRender();

    m_postProcessShader->SetShaderParameter(
        "UseAcesTonemapper",
        ParameterVariant((uint) m_params.Method));
  }

} // namespace ToolKit