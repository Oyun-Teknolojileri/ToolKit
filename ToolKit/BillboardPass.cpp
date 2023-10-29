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

#include "BillboardPass.h"

#include "Entity.h"
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

    renderer->SetFramebuffer(vp->m_framebuffer, false);
    CameraPtr cam           = vp->GetCamera();

    auto renderBillboardsFn = [this, cam, renderer](EntityPtrArray& billboards) -> void
    {
      m_jobs.clear();
      RenderJobProcessor::CreateRenderJobs(billboards, m_jobs);

      m_opaque.clear();
      m_translucent.clear();
      RenderJobProcessor::SeperateOpaqueTranslucent(m_jobs, m_opaque, m_translucent);

      auto renderArrayFn = [cam, renderer](RenderJobArray& jobs) -> void
      {
        for (RenderJob& rj : jobs)
        {
          renderer->Render(rj, cam);
        }
      };

      renderArrayFn(m_opaque);
      renderArrayFn(m_translucent);
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