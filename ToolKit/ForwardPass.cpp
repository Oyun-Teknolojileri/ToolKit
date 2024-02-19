/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
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

    RenderOpaque(m_params.renderData);
    RenderTranslucent(m_params.renderData);

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

    renderer->SetFramebuffer(m_params.FrameBuffer, m_params.clearBuffer);
    renderer->SetDepthTestFunc(CompareFunctions::FuncLequal);
    renderer->SetCamera(m_params.Cam, true);
    renderer->SetLights(m_params.Lights);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void ForwardRenderPass::PostRender()
  {
    PUSH_GPU_MARKER("ForwardRenderPass::PostRender");
    PUSH_CPU_MARKER("ForwardRenderPass::PostRender");

    Pass::PostRender();
    Renderer* renderer = GetRenderer();
    renderer->SetDepthTestFunc(CompareFunctions::FuncLess);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void ForwardRenderPass::RenderOpaque(RenderData* renderData)
  {
    PUSH_CPU_MARKER("ForwardRenderPass::RenderOpaque");

    Renderer* renderer = GetRenderer();

    if (m_params.SsaoTexture)
    {
      renderer->SetTexture(5, m_params.SsaoTexture->m_textureId);
    }

    RenderJobItr begin       = renderData->GetForwardOpaqueBegin();
    RenderJobItr end         = renderData->GetForwardTranslucentBegin();

    const MaterialPtr mat    = GetMaterialManager()->GetDefaultMaterial();
    GpuProgramPtr gpuProgram = GetGpuProgramManager()->CreateProgram(mat->m_vertexShader, mat->m_fragmentShader);

    for (RenderJobItr job = begin; job != end; job++)
    {
      if (job->Material->IsA<ShaderMaterial>())
      {
        renderer->RenderWithProgramFromMaterial(*job);
      }
      else
      {
        renderer->BindProgram(gpuProgram);
        renderer->Render(*job);
      }
    }

    POP_CPU_MARKER();
  }

  void ForwardRenderPass::RenderTranslucent(RenderData* renderData)
  {
    PUSH_CPU_MARKER("ForwardRenderPass::RenderTranslucent");

    RenderJobItr begin = renderData->GetForwardTranslucentBegin();
    RenderJobItr end   = renderData->jobs.end();

    RenderJobProcessor::SortByDistanceToCamera(begin, end, m_params.Cam);

    Renderer* renderer = GetRenderer();
    auto renderFnc     = [&](RenderJob& job)
    {
      Material* mat = job.Material;
      if (mat->GetRenderState()->cullMode == CullingType::TwoSided)
      {
        mat->GetRenderState()->cullMode = CullingType::Front;
        renderer->Render(job);

        mat->GetRenderState()->cullMode = CullingType::Back;
        renderer->Render(job);

        mat->GetRenderState()->cullMode = CullingType::TwoSided;
      }
      else
      {
        renderer->Render(job);
      }
    };

    const MaterialPtr defualtMat = GetMaterialManager()->GetDefaultMaterial();
    GpuProgramPtr defaultProgram =
        GetGpuProgramManager()->CreateProgram(defualtMat->m_vertexShader, defualtMat->m_fragmentShader);

    renderer->EnableDepthWrite(false);
    for (RenderJobArray::iterator job = begin; job != end; job++)
    {
      if (job->Material->IsA<ShaderMaterial>())
      {
        renderer->BindProgramOfMaterial(job->Material);
      }
      else
      {
        renderer->BindProgram(defaultProgram);
      }
      renderFnc(*job);
    }
    renderer->EnableDepthWrite(true);

    POP_CPU_MARKER();
  }

} // namespace ToolKit