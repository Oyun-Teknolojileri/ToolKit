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

#include "ForwardPass.h"

#include "Material.h"
#include "Mesh.h"
#include "Pass.h"
#include "Shader.h"
#include "TKProfiler.h"
#include "ToolKit.h"

namespace ToolKit
{

  ForwardRenderPass::ForwardRenderPass() {}

  ForwardRenderPass::ForwardRenderPass(const ForwardRenderPassParams& params) : m_params(params)
  {
    // Create a default frame buffer.
    if (m_params.FrameBuffer == nullptr)
    {
      m_params.FrameBuffer = MakeNewPtr<Framebuffer>();
      m_params.FrameBuffer->Init({1024u, 768u, false, true});
    }
  }

  void ForwardRenderPass::Render()
  {
    PUSH_GPU_MARKER("ForwardRenderPass::Render");
    PUSH_CPU_MARKER("ForwardRenderPass::Render");

    RenderOpaque(m_params.OpaqueJobs, m_params.Cam, m_params.Lights);
    RenderTranslucent(m_params.TranslucentJobs, m_params.Cam, m_params.Lights);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  ForwardRenderPass::~ForwardRenderPass() {}

  void ForwardRenderPass::PreRender()
  {
    PUSH_GPU_MARKER("ForwardRenderPass::PreRender");
    PUSH_CPU_MARKER("ForwardRenderPass::PreRender");

    Pass::PreRender();
    // Set self data.
    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_params.FrameBuffer, m_params.ClearFrameBuffer);
    if (!m_params.ClearFrameBuffer && m_params.ClearDepthBuffer)
    {
      renderer->ClearBuffer(GraphicBitFields::DepthStencilBits, Vec4(1.0f));
    }
    renderer->SetCameraLens(m_params.Cam);
    renderer->SetDepthTestFunc(CompareFunctions::FuncLequal);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void ForwardRenderPass::PostRender()
  {
    PUSH_GPU_MARKER("ForwardRenderPass::PostRender");
    PUSH_CPU_MARKER("ForwardRenderPass::PostRender");

    Pass::PostRender();
    GetRenderer()->m_overrideMat = nullptr;
    Renderer* renderer           = GetRenderer();
    renderer->SetDepthTestFunc(CompareFunctions::FuncLess);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void ForwardRenderPass::RenderOpaque(RenderJobArray& jobs, CameraPtr cam, LightPtrArray& lights)
  {
    PUSH_CPU_MARKER("ForwardRenderPass::RenderOpaque");

    Renderer* renderer = GetRenderer();

    if (m_params.SsaoTexture)
    {
      renderer->SetTexture(5, m_params.SsaoTexture->m_textureId);
    }

    for (const RenderJob& job : jobs)
    {
      RenderJobProcessor::SortLights(job, lights);
      job.Material->m_fragmentShader->SetShaderParameter("aoEnabled", ParameterVariant(m_params.SSAOEnabled));
      renderer->Render(job, m_params.Cam, lights);
    }

    POP_CPU_MARKER();
  }

  void ForwardRenderPass::RenderTranslucent(RenderJobArray& jobs, CameraPtr cam, LightPtrArray& lights)
  {
    PUSH_CPU_MARKER("ForwardRenderPass::RenderTranslucent");

    RenderJobProcessor::SortByDistanceToCamera(jobs, cam);

    Renderer* renderer = GetRenderer();
    auto renderFnc     = [cam, &lights, renderer](RenderJob& job)
    {
      RenderJobProcessor::SortLights(job, lights);

      MaterialPtr mat = job.Material;
      if (mat->GetRenderState()->cullMode == CullingType::TwoSided)
      {
        mat->GetRenderState()->cullMode = CullingType::Front;
        renderer->Render(job, cam, lights);

        mat->GetRenderState()->cullMode = CullingType::Back;
        renderer->Render(job, cam, lights);

        mat->GetRenderState()->cullMode = CullingType::TwoSided;
      }
      else
      {
        renderer->Render(job, cam, lights);
      }
    };

    for (RenderJob& job : jobs)
    {
      renderFnc(job);
    }

    POP_CPU_MARKER();
  }

} // namespace ToolKit