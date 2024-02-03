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
#include "TKProfiler.h"
#include "ToolKit.h"

namespace ToolKit
{

  StencilRenderPass::StencilRenderPass()
  {
    // Init sub pass.
    m_copyStencilSubPass = MakeNewPtr<FullQuadPass>();
    m_copyStencilSubPass->m_params.FragmentShader =
        GetShaderManager()->Create<Shader>(ShaderPath("unlitFrag.shader", true));
    m_frameBuffer           = MakeNewPtr<Framebuffer>();

    m_solidOverrideMaterial = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
  }

  StencilRenderPass::StencilRenderPass(const StencilRenderPassParams& params) : StencilRenderPass()
  {
    m_params = params;
  }

  StencilRenderPass::~StencilRenderPass()
  {
    m_frameBuffer           = nullptr;
    m_solidOverrideMaterial = nullptr;
    m_copyStencilSubPass    = nullptr;
  }

  void StencilRenderPass::Render()
  {
    PUSH_GPU_MARKER("StencilRenderPass::Render");
    PUSH_CPU_MARKER("StencilRenderPass::Render");

    Renderer* renderer      = GetRenderer();
    renderer->m_overrideMat = m_solidOverrideMaterial;

    // Stencil pass.
    renderer->SetStencilOperation(StencilOperation::AllowAllPixels);
    renderer->ColorMask(false, false, false, false);

    renderer->Render(m_params.RenderJobs);

    // Copy pass.
    renderer->ColorMask(true, true, true, true);
    renderer->SetStencilOperation(StencilOperation::AllowPixelsFailingStencil);

    RenderSubPass(m_copyStencilSubPass);

    renderer->SetStencilOperation(StencilOperation::None);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void StencilRenderPass::PreRender()
  {
    PUSH_GPU_MARKER("StencilRenderPass::PreRender");
    PUSH_CPU_MARKER("StencilRenderPass::PreRender");

    Pass::PreRender();
    Renderer* renderer = GetRenderer();

    FramebufferSettings settings;
    settings.depthStencil    = true;
    settings.useDefaultDepth = true;
    settings.width           = m_params.OutputTarget->m_width;
    settings.height          = m_params.OutputTarget->m_height;

    m_frameBuffer->Init(settings);
    m_frameBuffer->ReconstructIfNeeded(settings.width, settings.height);
    m_frameBuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_params.OutputTarget);
    m_copyStencilSubPass->m_params.FrameBuffer      = m_frameBuffer;
    m_copyStencilSubPass->m_params.ClearFrameBuffer = false;

    // Allow writing on to stencil before clear operation.
    renderer->SetStencilOperation(StencilOperation::AllowAllPixels);
    renderer->SetFramebuffer(m_frameBuffer, GraphicBitFields::AllBits);
    renderer->SetCamera(m_params.Camera, true);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void StencilRenderPass::PostRender()
  {
    PUSH_GPU_MARKER("StencilRenderPass::PostRender");
    PUSH_CPU_MARKER("StencilRenderPass::PostRender");

    Pass::PostRender();

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

} // namespace ToolKit