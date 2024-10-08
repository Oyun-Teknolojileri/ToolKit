/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "DofPass.h"

#include "Shader.h"
#include "ToolKit.h"

namespace ToolKit
{

  DoFPass::DoFPass() : Pass("DoFPass")
  {
    m_quadPass                       = MakeNewPtr<FullQuadPass>();
    m_quadPass->m_params.frameBuffer = MakeNewPtr<Framebuffer>("DofFB");
    m_dofShader                      = GetShaderManager()->Create<Shader>(ShaderPath("depthOfFieldFrag.shader", true));
    m_copyTexture                    = MakeNewPtr<RenderTarget>();
  }

  DoFPass::DoFPass(const DoFPassParams& params) : DoFPass() { m_params = params; }

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

    const TextureSettings& colorRTSet = m_params.ColorRt->Settings();
    m_copyTexture->ReconstructIfNeeded(m_params.ColorRt->m_width, m_params.ColorRt->m_height, &colorRTSet);

    GetRenderer()->CopyTexture(m_params.ColorRt, m_copyTexture);

    m_quadPass->SetFragmentShader(m_dofShader, GetRenderer());

    m_quadPass->UpdateUniform(ShaderUniform("focusPoint", m_params.focusPoint));
    m_quadPass->UpdateUniform(ShaderUniform("focusScale", m_params.focusScale));
    m_quadPass->UpdateUniform(ShaderUniform("blurSize", 5.0f));

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
    m_quadPass->UpdateUniform(ShaderUniform("radiusScale", blurRadiusScale));

    IVec2 size(m_params.ColorRt->m_width, m_params.ColorRt->m_height);

    m_quadPass->m_params.frameBuffer->ReconstructIfNeeded({size.x, size.y, false, false});
    m_quadPass->UpdateUniform(ShaderUniform("uPixelSize", Vec2(1.0f) / Vec2(size)));
    m_quadPass->m_params.blendFunc        = BlendFunction::NONE;
    m_quadPass->m_params.clearFrameBuffer = GraphicBitFields::None;
    m_quadPass->m_params.frameBuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_params.ColorRt);
  }

  void DoFPass::Render()
  {
    Renderer* renderer = GetRenderer();
    if (m_params.ColorRt == nullptr)
    {
      return;
    }

    renderer->SetTexture(0, m_copyTexture->m_textureId);
    renderer->SetTexture(1, m_params.DepthRt->m_textureId);

    RenderSubPass(m_quadPass);
  }

  void DoFPass::PostRender() { Pass::PostRender(); }

} // namespace ToolKit