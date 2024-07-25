/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "PostProcessPass.h"

#include "Shader.h"
#include "TKProfiler.h"
#include "ToolKit.h"

namespace ToolKit
{

  PostProcessPass::PostProcessPass()
  {
    m_copyTexture = MakeNewPtr<RenderTarget>();
    m_copyBuffer  = MakeNewPtr<Framebuffer>();
    m_copyBuffer->Init({0, 0, false, false});

    m_postProcessPass = MakeNewPtr<FullQuadPass>();
  }

  PostProcessPass::PostProcessPass(const PostProcessPassParams& params) : PostProcessPass() { m_params = params; }

  PostProcessPass::~PostProcessPass()
  {
    m_postProcessShader = nullptr;
    m_postProcessPass   = nullptr;
    m_copyBuffer        = nullptr;
    m_copyTexture       = nullptr;
  }

  void PostProcessPass::PreRender()
  {
    PUSH_GPU_MARKER("PostProcessPass::PreRender");
    PUSH_CPU_MARKER("PostProcessPass::PreRender");

    Pass::PreRender();

    RenderTargetPtr srcTexture = m_params.FrameBuffer->GetColorAttachment(Framebuffer::Attachment::ColorAttachment0);
    m_copyTexture->ReconstructIfNeeded(srcTexture->m_width, srcTexture->m_height, &srcTexture->Settings());

    Renderer* renderer = GetRenderer();
    renderer->CopyTexture(srcTexture, m_copyTexture);

    // Set given buffer as a texture to be read in post process pass.
    renderer->SetTexture(0, m_copyTexture->m_textureId);

    m_postProcessPass->SetFragmentShader(m_postProcessShader, renderer);
    m_postProcessPass->m_params.frameBuffer      = m_params.FrameBuffer;
    m_postProcessPass->m_params.clearFrameBuffer = GraphicBitFields::None;

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void PostProcessPass::Render()
  {
    PUSH_GPU_MARKER("PostProcessPass::Render");
    PUSH_CPU_MARKER("PostProcessPass::Render");

    RenderSubPass(m_postProcessPass);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void PostProcessPass::PostRender()
  {
    PUSH_GPU_MARKER("PostProcessPass::PostRender");
    PUSH_CPU_MARKER("PostProcessPass::PostRender");

    Pass::PostRender();

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

} // namespace ToolKit