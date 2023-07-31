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

#include "StencilPass.h"

#include "Material.h"
#include "Mesh.h"
#include "ToolKit.h"

namespace ToolKit
{

  StencilRenderPass::StencilRenderPass()
  {
    // Init sub pass.
    m_copyStencilSubPass = std::make_shared<FullQuadPass>();
    m_copyStencilSubPass->m_params.FragmentShader =
        GetShaderManager()->Create<Shader>(ShaderPath("unlitFrag.shader", true));
    
    m_frameBuffer = std::make_shared<Framebuffer>();

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
    Renderer* renderer      = GetRenderer();
    renderer->m_overrideMat = m_solidOverrideMaterial;

    // Stencil pass.
    renderer->SetStencilOperation(StencilOperation::AllowAllPixels);
    renderer->ColorMask(false, false, false, false);

    for (RenderJob& job : m_params.RenderJobs)
    {
      renderer->Render(job, m_params.Camera);
    }

    // Copy pass.
    renderer->ColorMask(true, true, true, true);
    renderer->SetStencilOperation(StencilOperation::AllowPixelsFailingStencil);

    RenderSubPass(m_copyStencilSubPass);

    renderer->SetStencilOperation(StencilOperation::None);
  }

  void StencilRenderPass::PreRender()
  {
    Pass::PreRender();

    FramebufferSettings settings;
    settings.depthStencil    = true;
    settings.useDefaultDepth = true;
    settings.width           = m_params.OutputTarget->m_width;
    settings.height          = m_params.OutputTarget->m_height;

    m_frameBuffer->Init(settings);
    m_frameBuffer->ReconstructIfNeeded(settings.width, settings.height);
    m_frameBuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0, m_params.OutputTarget);

    m_copyStencilSubPass->m_params.FrameBuffer = m_frameBuffer;

    Renderer* renderer                         = GetRenderer();
    renderer->SetFramebuffer(m_frameBuffer, true, Vec4(0.0f));
    renderer->SetCameraLens(m_params.Camera);
  }

  void StencilRenderPass::PostRender() { Pass::PostRender(); }

} // namespace ToolKit