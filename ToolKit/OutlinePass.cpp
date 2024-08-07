/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "OutlinePass.h"

#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "TKProfiler.h"
#include "ToolKit.h"

namespace ToolKit
{

  OutlinePass::OutlinePass()
  {
    m_stencilPass  = MakeNewPtr<StencilRenderPass>();
    m_stencilAsRt  = MakeNewPtr<RenderTarget>();

    m_outlinePass  = MakeNewPtr<FullQuadPass>();
    m_dilateShader = GetShaderManager()->Create<Shader>(ShaderPath("dilateFrag.shader", true));
  }

  OutlinePass::OutlinePass(const OutlinePassParams& params) : OutlinePass() { m_params = params; }

  OutlinePass::~OutlinePass()
  {
    m_stencilPass  = nullptr;
    m_outlinePass  = nullptr;
    m_dilateShader = nullptr;
    m_stencilAsRt  = nullptr;
  }

  void OutlinePass::Render()
  {
    assert(m_params.RenderJobs != nullptr && "Outline Pass Render Jobs Are Not Given!");

    // Generate stencil binary image.
    RenderSubPass(m_stencilPass);

    // Use stencil output as input to the dilation.
    GetRenderer()->SetTexture(0, m_stencilAsRt->m_textureId);

    m_outlinePass->SetFragmentShader(m_dilateShader, GetRenderer());
    m_outlinePass->UpdateUniform(ShaderUniform("Color", m_params.OutlineColor));

    // Draw outline to the viewport.
    m_outlinePass->m_params.frameBuffer      = m_params.FrameBuffer;
    m_outlinePass->m_params.clearFrameBuffer = GraphicBitFields::None;

    RenderSubPass(m_outlinePass);
  }

  void OutlinePass::PreRender()
  {
    Pass::PreRender();

    // Create stencil image.
    m_stencilPass->m_params.Camera     = m_params.Camera;
    m_stencilPass->m_params.RenderJobs = m_params.RenderJobs;

    // Construct output target.
    const FramebufferSettings& fbs     = m_params.FrameBuffer->GetSettings();
    m_stencilAsRt->ReconstructIfNeeded(fbs.width, fbs.height);
    m_stencilPass->m_params.OutputTarget = m_stencilAsRt;
  }

  void OutlinePass::PostRender() { Pass::PostRender(); }

} // namespace ToolKit