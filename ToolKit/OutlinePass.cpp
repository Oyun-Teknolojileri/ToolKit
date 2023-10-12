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

#include "OutlinePass.h"

#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "ToolKit.h"

#define NOMINMAX
#include "nvtx3.hpp"
#undef WriteConsole

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
    nvtxRangePushA("OutlinePass Render");

    // Generate stencil binary image.
    RenderSubPass(m_stencilPass);

    // Use stencil output as input to the dilation.
    GetRenderer()->SetTexture(0, m_stencilAsRt->m_textureId);
    m_dilateShader->SetShaderParameter("Color", ParameterVariant(m_params.OutlineColor));

    // Draw outline to the viewport.
    m_outlinePass->m_params.FragmentShader   = m_dilateShader;
    m_outlinePass->m_params.FrameBuffer      = m_params.FrameBuffer;
    m_outlinePass->m_params.ClearFrameBuffer = false;

    RenderSubPass(m_outlinePass);

    nvtxRangePop();
  }

  void OutlinePass::PreRender()
  {
    nvtxRangePushA("OutlinePass PreRender");

    Pass::PreRender();

    // Create stencil image.
    m_stencilPass->m_params.Camera     = m_params.Camera;
    m_stencilPass->m_params.RenderJobs = m_params.RenderJobs;

    // Construct output target.
    FramebufferSettings fbs            = m_params.FrameBuffer->GetSettings();
    m_stencilAsRt->ReconstructIfNeeded(fbs.width, fbs.height);
    m_stencilPass->m_params.OutputTarget = m_stencilAsRt;
    
    nvtxRangePop();
  }

  void OutlinePass::PostRender()
  {
    nvtxRangePushA("OutlinePass PostRender");

    Pass::PostRender();
    
    nvtxRangePop();
  }

} // namespace ToolKit