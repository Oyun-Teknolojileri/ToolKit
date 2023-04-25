#include "GizmoPass.h"

#include "Material.h"
#include "Mesh.h"
#include "ResourceComponent.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    GizmoPass::GizmoPass()
    {
      m_depthMaskSphere   = std::make_shared<Sphere>(0.95f);
      MeshComponentPtr mc = m_depthMaskSphere->GetMeshComponent();
      MeshPtr mesh        = mc->GetMeshVal();
      RenderState* rs     = mesh->m_material->GetRenderState();
      rs->cullMode        = CullingType::Front;
    }

    GizmoPass::GizmoPass(const GizmoPassParams& params) : GizmoPass()
    {
      m_params = params;
    }

    void GizmoPass::Render()
    {
      Renderer* renderer = GetRenderer();

      for (EditorBillboardBase* bb : m_params.GizmoArray)
      {
        if (bb->GetBillboardType() ==
            EditorBillboardBase::BillboardType::Rotate)
        {
          Mat4 ts = bb->m_node->GetTransform();
          m_depthMaskSphere->m_node->SetTransform(ts,
                                                  TransformationSpace::TS_WORLD,
                                                  false);

          renderer->ColorMask(false, false, false, false);

          RenderJobArray jobs;
          RenderJobProcessor::CreateRenderJob(m_depthMaskSphere.get(), jobs);
          renderer->Render(jobs, m_camera);

          renderer->ColorMask(true, true, true, true);

          jobs.clear();
          RenderJobProcessor::CreateRenderJob(bb, jobs);
          renderer->Render(jobs, m_camera);
        }
        else
        {
          RenderJobArray jobs;
          RenderJobProcessor::CreateRenderJob(bb, jobs);
          renderer->Render(jobs, m_camera);
        }
      }
    }

    void GizmoPass::PreRender()
    {
      Pass::PreRender();

      Renderer* renderer = GetRenderer();
      m_camera           = m_params.Viewport->GetCamera();
      renderer->SetFramebuffer(m_params.Viewport->m_framebuffer, false);
      renderer->SetCameraLens(m_camera);
      renderer->ClearBuffer(GraphicBitFields::DepthBits, Vec4(1.0f));

      // Update.
      BillboardRawPtrArray& gizmoArray = m_params.GizmoArray;
      gizmoArray.erase(
          std::remove_if(gizmoArray.begin(),
                         gizmoArray.end(),
                         [this](EditorBillboardBase* bb) -> bool
                         {
                           if (bb == nullptr)
                           {
                             return true;
                           }

                           bb->LookAt(m_camera,
                                      m_params.Viewport->GetBillboardScale());
                           return false;
                         }),
          gizmoArray.end());
    }

    void GizmoPass::PostRender() { Pass::PostRender(); }

  } // namespace Editor
} // namespace ToolKit