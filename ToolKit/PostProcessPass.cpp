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

#include "DebugNew.h"

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

    Renderer* renderer = GetRenderer();

    // Initiate copy buffer.
    FramebufferSettings fbs;
    fbs.depthStencil    = false;
    fbs.useDefaultDepth = false;
    if (m_params.FrameBuffer == nullptr)
    {
      fbs.width  = renderer->m_windowSize.x;
      fbs.height = renderer->m_windowSize.y;
    }
    else
    {
      const FramebufferSettings& targetFbs = m_params.FrameBuffer->GetSettings();
      fbs.width                            = targetFbs.width;
      fbs.height                           = targetFbs.height;
    }

    m_copyTexture->ReconstructIfNeeded(fbs.width, fbs.height);
    m_copyBuffer->ReconstructIfNeeded(fbs.width, fbs.height);
    m_copyBuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_copyTexture);

    // Copy given buffer.
    renderer->CopyFrameBuffer(m_params.FrameBuffer, m_copyBuffer, GraphicBitFields::ColorBits);

    // Set given buffer as a texture to be read in gamma pass.
    renderer->SetTexture(0, m_copyTexture->m_textureId);

    m_postProcessPass->SetFragmentShader(m_postProcessShader, renderer);
    m_postProcessPass->m_params.FrameBuffer      = m_params.FrameBuffer;
    m_postProcessPass->m_params.ClearFrameBuffer = false;

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