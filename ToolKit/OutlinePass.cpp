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

    PUSH_GPU_MARKER("OutlinePass::Render");
    PUSH_CPU_MARKER("OutlinePass::Render");

    // Generate stencil binary image.
    RenderSubPass(m_stencilPass);

    // Use stencil output as input to the dilation.
    GetRenderer()->SetTexture(0, m_stencilAsRt->m_textureId);
    m_dilateShader->UpdateShaderUniform("Color", m_params.OutlineColor);

    // Draw outline to the viewport.
    m_outlinePass->m_params.FragmentShader   = m_dilateShader;
    m_outlinePass->m_params.FrameBuffer      = m_params.FrameBuffer;
    m_outlinePass->m_params.ClearFrameBuffer = false;

    RenderSubPass(m_outlinePass);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void OutlinePass::PreRender()
  {
    PUSH_GPU_MARKER("OutlinePass::PreRender");
    PUSH_CPU_MARKER("OutlinePass::PreRender");

    Pass::PreRender();

    // Create stencil image.
    m_stencilPass->m_params.Camera     = m_params.Camera;
    m_stencilPass->m_params.RenderJobs = m_params.RenderJobs;

    // Construct output target.
    const FramebufferSettings& fbs     = m_params.FrameBuffer->GetSettings();
    m_stencilAsRt->ReconstructIfNeeded(fbs.width, fbs.height);
    m_stencilPass->m_params.OutputTarget = m_stencilAsRt;

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void OutlinePass::PostRender()
  {
    PUSH_GPU_MARKER("OutlinePass::PostRender");
    PUSH_CPU_MARKER("OutlinePass::PostRender");

    Pass::PostRender();

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

} // namespace ToolKit