/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "PostProcessPass.h"

#include "Shader.h"
#include "ShaderReflectionCache.h"
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
      FramebufferSettings targetFbs = m_params.FrameBuffer->GetSettings();
      fbs.width                     = targetFbs.width;
      fbs.height                    = targetFbs.height;
    }

    m_copyTexture->ReconstructIfNeeded(fbs.width, fbs.height);
    m_copyBuffer->ReconstructIfNeeded(fbs.width, fbs.height);
    m_copyBuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_copyTexture);

    // Copy given buffer.
    renderer->CopyFrameBuffer(m_params.FrameBuffer, m_copyBuffer, GraphicBitFields::ColorBits);

    // Set given buffer as a texture to be read in gamma pass.
    renderer->SetTexture(0, m_copyTexture->m_textureId);

    m_postProcessPass->m_params.FragmentShader   = m_postProcessShader;
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