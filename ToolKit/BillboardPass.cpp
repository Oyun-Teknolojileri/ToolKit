/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "BillboardPass.h"

#include "Entity.h"
#include "Material.h"
#include "TKProfiler.h"

#include "DebugNew.h"

namespace ToolKit
{
  BillboardPass::BillboardPass() {}

  BillboardPass::BillboardPass(const BillboardPassParams& params) {}

  BillboardPass::~BillboardPass() {}

  void BillboardPass::Render()
  {
    PUSH_GPU_MARKER("BillboardPass::Render");
    PUSH_CPU_MARKER("BillboardPass::Render");

    Renderer* renderer = GetRenderer();
    Viewport* vp       = m_params.Viewport;

    renderer->SetFramebuffer(vp->m_framebuffer, GraphicBitFields::None);
    CameraPtr cam = vp->GetCamera();
    renderer->SetCamera(cam, true);

    GpuProgramManager* gpuProgramManager = GetGpuProgramManager();

    auto renderBillboardsFn              = [this, cam, renderer, gpuProgramManager](EntityPtrArray& billboards) -> void
    {
      m_renderData.jobs.clear();
      RenderJobProcessor::CreateRenderJobs(billboards, m_renderData.jobs);
      RenderJobProcessor::SeperateRenderData(m_renderData, true);

      renderer->RenderWithProgramFromMaterial(m_renderData.jobs);
    };

    renderer->EnableDepthTest(false);
    renderBillboardsFn(m_noDepthBillboards);

    renderer->EnableDepthTest(true);
    renderBillboardsFn(m_params.Billboards);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void BillboardPass::PreRender()
  {
    PUSH_GPU_MARKER("BillboardPass::PreRender");
    PUSH_CPU_MARKER("BillboardPass::PreRender");

    Pass::PreRender();

    // Process billboards.
    float vpScale = m_params.Viewport->GetBillboardScale();
    CameraPtr cam = m_params.Viewport->GetCamera();
    m_noDepthBillboards.clear();

    // Separate functions that does not require depth test.
    move_values(m_params.Billboards,
                m_noDepthBillboards,
                [this, vpScale, cam](EntityPtr bb) -> bool
                {
                  // Update billboards.
                  assert(bb->IsA<Billboard>());
                  BillboardPtr cbb = std::static_pointer_cast<Billboard>(bb);
                  cbb->LookAt(cam, vpScale);

                  // Return separation condition.
                  return cbb->m_settings.bypassDepthTest;
                });

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

} // namespace ToolKit