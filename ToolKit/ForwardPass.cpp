#include "ForwardPass.h"

#include "Material.h"
#include "Mesh.h"
#include "ToolKit.h"
#include "Pass.h"

namespace ToolKit
{

  ForwardRenderPass::ForwardRenderPass() {}

  ForwardRenderPass::ForwardRenderPass(const ForwardRenderPassParams& params)
      : m_params(params)
  {
    // Create a default frame buffer.
    if (m_params.FrameBuffer == nullptr)
    {
      m_params.FrameBuffer = std::make_shared<Framebuffer>();
      m_params.FrameBuffer->Init({1024u, 768u, false, true});
    }
  }

  void ForwardRenderPass::Render()
  {
    RenderOpaque(m_params.OpaqueJobs, m_params.Cam, m_params.Lights);
    RenderTranslucent(m_params.TranslucentJobs, m_params.Cam, m_params.Lights);
  }

  ForwardRenderPass::~ForwardRenderPass() {}

  void ForwardRenderPass::PreRender()
  {
    Pass::PreRender();

    // Set self data.
    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_params.FrameBuffer, m_params.ClearFrameBuffer);
    renderer->SetCameraLens(m_params.Cam);
  }

  void ForwardRenderPass::PostRender()
  {
    Pass::PostRender();
    GetRenderer()->m_overrideMat = nullptr;
  }

  void ForwardRenderPass::RenderOpaque(RenderJobArray jobs,
                                       Camera* cam,
                                       const LightRawPtrArray& lights)
  {
    Renderer* renderer = GetRenderer();
    for (RenderJob& job : jobs)
    {
      LightRawPtrArray lightList = RenderJobProcessor::SortLights(job, lights);
      uint activeMeshIndx        = 0;
      renderer->Render(job, m_params.Cam, lightList);
    }
  }

  void ForwardRenderPass::RenderTranslucent(RenderJobArray jobs,
                                            Camera* cam,
                                            const LightRawPtrArray& lights)
  {
    RenderJobProcessor::StableSortByDistanceToCamera(jobs, cam);

    Renderer* renderer = GetRenderer();
    auto renderFnc     = [cam, lights, renderer](RenderJob& job)
    {
      LightRawPtrArray culledLights =
          RenderJobProcessor::SortLights(job, lights);

      MaterialPtr mat = job.Material;
      if (mat->GetRenderState()->cullMode == CullingType::TwoSided)
      {
        mat->GetRenderState()->cullMode = CullingType::Front;
        renderer->Render(job, cam, culledLights);

        mat->GetRenderState()->cullMode = CullingType::Back;
        renderer->Render(job, cam, culledLights);

        mat->GetRenderState()->cullMode = CullingType::TwoSided;
      }
      else
      {
        renderer->Render(job, cam, culledLights);
      }
    };

    for (RenderJob& job : jobs)
    {
      renderFnc(job);
    }
  }

} // namespace ToolKit