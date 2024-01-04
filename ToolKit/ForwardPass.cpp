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

    renderer->SetFramebuffer(m_params.FrameBuffer, m_params.clearBuffer);
    renderer->SetCamera(m_params.Cam, true);
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

    int dirLightEnd = RenderJobProcessor::PreSortLights(lights);

    LightPtrArray activeLights;
    activeLights.reserve(Renderer::RHIConstants::MaxLightsPerObject);

    for (const RenderJob& job : jobs)
    {
      int effectiveLights = RenderJobProcessor::SortLights(job, lights, dirLightEnd);
      job.Material->m_fragmentShader->UpdateShaderUniform("aoEnabled", m_params.SSAOEnabled);

      activeLights.insert(activeLights.begin(), lights.begin(), lights.begin() + effectiveLights);
      renderer->Render(job, m_params.Cam, activeLights);
      activeLights.clear();
    }

    POP_CPU_MARKER();
  }

  void ForwardRenderPass::RenderTranslucent(RenderJobArray& jobs, CameraPtr cam, LightPtrArray& lights)
  {
    PUSH_CPU_MARKER("ForwardRenderPass::RenderTranslucent");

    RenderJobProcessor::SortByDistanceToCamera(jobs, cam);

    int dirLightEnd    = RenderJobProcessor::PreSortLights(lights);

    Renderer* renderer = GetRenderer();
    auto renderFnc     = [cam, &lights, dirLightEnd, renderer](RenderJob& job)
    {
      RenderJobProcessor::SortLights(job, lights, dirLightEnd);

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

    renderer->EnableDepthWrite(false);
    for (RenderJob& job : jobs)
    {
      renderFnc(job);
    }
    renderer->EnableDepthWrite(true);

    POP_CPU_MARKER();
  }

} // namespace ToolKit