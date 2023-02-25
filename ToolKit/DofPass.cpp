#include "DofPass.h"

#include "Shader.h"
#include "ShaderReflectionCache.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  DoFPass::DoFPass()
  {
    m_quadPass                       = std::make_shared<FullQuadPass>();
    m_quadPass->m_params.FrameBuffer = std::make_shared<Framebuffer>();

    m_dofShader                      = GetShaderManager()->Create<Shader>(
        ShaderPath("depthOfFieldFrag.shader", true));
  }

  DoFPass::DoFPass(const DoFPassParams& params) : DoFPass()
  {
    m_params = params;
  }

  DoFPass::~DoFPass()
  {
    m_dofShader = nullptr;
    m_quadPass  = nullptr;
  }

  void DoFPass::PreRender()
  {
    Pass::PreRender();
    if (m_params.ColorRt == nullptr)
    {
      return;
    }

    m_dofShader->SetShaderParameter("focusPoint",
                                    ParameterVariant(m_params.focusPoint));
    m_dofShader->SetShaderParameter("focusScale",
                                    ParameterVariant(m_params.focusScale));
    m_dofShader->SetShaderParameter("blurSize", ParameterVariant(5.0f));

    float blurRadiusScale = 0.5f;
    switch (m_params.blurQuality)
    {
    case DoFQuality::Low:
      blurRadiusScale = 2.0f;
      break;
    case DoFQuality::Normal:
      blurRadiusScale = 0.7f;
      break;
    case DoFQuality::High:
      blurRadiusScale = 0.2f;
      break;
    }
    m_dofShader->SetShaderParameter("radiusScale",
                                    ParameterVariant(blurRadiusScale));

    UVec2 size(m_params.ColorRt->m_width, m_params.ColorRt->m_height);

    m_quadPass->m_params.FrameBuffer->Init({size.x, size.y, false, false});
    m_dofShader->SetShaderParameter("uPixelSize",
                                    ParameterVariant(Vec2(1.0f) / Vec2(size)));
    m_quadPass->m_params.FrameBuffer->SetAttachment(
        Framebuffer::Attachment::ColorAttachment0,
        m_params.ColorRt);
    m_quadPass->m_params.BlendFunc        = BlendFunction::NONE;
    m_quadPass->m_params.ClearFrameBuffer = false;
    m_quadPass->m_params.FragmentShader   = m_dofShader;
    m_quadPass->m_params.lights           = {};
  }

  void DoFPass::Render()
  {
    Renderer* renderer = GetRenderer();
    if (m_params.ColorRt == nullptr)
    {
      return;
    }

    renderer->SetTexture(0, m_params.ColorRt->m_textureId);
    renderer->SetTexture(1, m_params.DepthRt->m_textureId);

    RenderSubPass(m_quadPass);
  }

  void DoFPass::PostRender() { Pass::PostRender(); }

} // namespace ToolKit