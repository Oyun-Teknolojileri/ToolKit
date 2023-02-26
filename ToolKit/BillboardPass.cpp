#include "BillboardPass.h"

#include "Entity.h"

#include "DebugNew.h"

namespace ToolKit
{
  BillboardPass::BillboardPass() {}

  BillboardPass::BillboardPass(const BillboardPassParams& params) {}

  BillboardPass::~BillboardPass() {}

  void BillboardPass::Render()
  {
    Renderer* renderer = GetRenderer();
    Viewport* vp       = m_params.Viewport;

    renderer->SetFramebuffer(vp->m_framebuffer, false);
    Camera* cam = vp->GetCamera();

    auto renderBillboardsFn =
        [this, cam, renderer](EntityRawPtrArray& billboards) -> void
    {
      RenderJobArray jobs;
      RenderJobProcessor::CreateRenderJobs(billboards, jobs);

      RenderJobArray opaque, translucent;
      RenderJobProcessor::SeperateOpaqueTranslucent(jobs, opaque, translucent);

      auto renderArrayFn = [cam, renderer](RenderJobArray& jobs) -> void
      {
        for (RenderJob& rj : jobs)
        {
          renderer->Render(rj, cam);
        }
      };

      renderArrayFn(opaque);
      renderArrayFn(translucent);
    };

    renderer->EnableDepthTest(false);
    renderBillboardsFn(m_noDepthBillboards);

    renderer->EnableDepthTest(true);
    renderBillboardsFn(m_params.Billboards);
  }

  void BillboardPass::PreRender()
  {
    Pass::PreRender();

    // Process billboards.
    float vpScale = m_params.Viewport->GetBillboardScale();
    Camera* cam   = m_params.Viewport->GetCamera();
    m_noDepthBillboards.clear();

    // Separate functions that does not require depth test.
    auto iter =
        std::remove_if(m_params.Billboards.begin(),
                       m_params.Billboards.end(),
                       [this, vpScale, cam](Entity* bb) -> bool
                       {
                         // Update billboards.
                         assert(bb->GetType() == EntityType::Entity_Billboard);
                         Billboard* cbb = static_cast<Billboard*>(bb);
                         cbb->LookAt(cam, vpScale);

                         // Return separation condition.
                         return cbb->m_settings.bypassDepthTest;
                       });
    m_noDepthBillboards.insert(m_noDepthBillboards.begin(),
                               iter,
                               m_params.Billboards.end());
    m_params.Billboards.erase(iter, m_params.Billboards.end());
  }

} // namespace ToolKit