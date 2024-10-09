/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "StencilPass.h"

#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "TKOpenGL.h"
#include "ToolKit.h"

namespace ToolKit
{

  StencilRenderPass::StencilRenderPass() : Pass("StencilRenderPass")
  {
    // Init sub pass.
    m_copyStencilSubPass    = MakeNewPtr<FullQuadPass>();
    m_unlitFragShader       = GetShaderManager()->Create<Shader>(ShaderPath("unlitFrag.shader", true));
    m_frameBuffer           = MakeNewPtr<Framebuffer>("StencilPassFB");

    m_solidOverrideMaterial = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
  }

  void StencilRenderPass::Render()
  {
    assert(m_params.RenderJobs != nullptr && "Stencil Render Pass Render Jobs Are Not Given!");

    Renderer* renderer = GetRenderer();

    // Stencil pass.
    renderer->SetStencilOperation(StencilOperation::AllowAllPixels);
    renderer->ColorMask(false, false, false, false);

    renderer->Render(*m_params.RenderJobs);

    // Copy pass.
    renderer->ColorMask(true, true, true, true);
    renderer->SetStencilOperation(StencilOperation::AllowPixelsFailingStencil);

    m_copyStencilSubPass->SetFragmentShader(m_unlitFragShader, renderer);

    RenderSubPass(m_copyStencilSubPass);

    renderer->SetStencilOperation(StencilOperation::None);
  }

  void StencilRenderPass::PreRender()
  {
    Pass::PreRender();
    Renderer* renderer                   = GetRenderer();

    GpuProgramManager* gpuProgramManager = GetGpuProgramManager();
    m_program                            = gpuProgramManager->CreateProgram(m_solidOverrideMaterial->m_vertexShader,
                                                 m_solidOverrideMaterial->m_fragmentShader);
    renderer->BindProgram(m_program);

    FramebufferSettings settings;
    settings.depthStencil    = true;
    settings.useDefaultDepth = true;
    settings.width           = m_params.OutputTarget->m_width;
    settings.height          = m_params.OutputTarget->m_height;

    m_frameBuffer->ReconstructIfNeeded(settings);
    m_frameBuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_params.OutputTarget);
    m_copyStencilSubPass->m_params.frameBuffer      = m_frameBuffer;
    m_copyStencilSubPass->m_params.clearFrameBuffer = GraphicBitFields::None;

    // Allow writing on to stencil before clear operation.
    renderer->SetStencilOperation(StencilOperation::AllowAllPixels);
    renderer->SetFramebuffer(m_frameBuffer, GraphicBitFields::AllBits);
    renderer->SetCamera(m_params.Camera, true);
  }

  void StencilRenderPass::PostRender() { Pass::PostRender(); }

} // namespace ToolKit