/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "GizmoPass.h"

#include "Material.h"
#include "Mesh.h"
#include "ResourceComponent.h"
#include "TKProfiler.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    GizmoPass::GizmoPass()
    {
      m_depthMaskSphere = MakeNewPtr<Sphere>();
      m_depthMaskSphere->SetRadiusVal(0.95f);

      MeshComponentPtr mc = m_depthMaskSphere->GetMeshComponent();
      MeshPtr mesh        = mc->GetMeshVal();
      RenderState* rs     = mesh->m_material->GetRenderState();
      rs->cullMode        = CullingType::Front;
    }

    GizmoPass::GizmoPass(const GizmoPassParams& params) : GizmoPass() { m_params = params; }

    void GizmoPass::Render()
    {
      PUSH_CPU_MARKER("GizmoPass::Render");

      Renderer* renderer = GetRenderer();

      for (EditorBillboardPtr billboard : m_params.GizmoArray)
      {
        if (billboard->GetBillboardType() == EditorBillboardBase::BillboardType::Rotate)
        {
          Mat4 ts = billboard->m_node->GetTransform();
          m_depthMaskSphere->m_node->SetTransform(ts, TransformationSpace::TS_WORLD);

          renderer->ColorMask(false, false, false, false);

          RenderJobArray jobs;
          RenderJobProcessor::CreateRenderJobs({m_depthMaskSphere}, jobs);
          renderer->Render(jobs);

          renderer->ColorMask(true, true, true, true);

          jobs.clear();
          RenderJobProcessor::CreateRenderJobs({billboard}, jobs);
          renderer->Render(jobs);
        }
        else
        {
          RenderJobArray jobs;
          RenderJobProcessor::CreateRenderJobs({billboard}, jobs);
          renderer->Render(jobs);
        }
      }

      POP_CPU_MARKER();
    }

    void GizmoPass::PreRender()
    {
      PUSH_CPU_MARKER("GizmoPass::PreRender");

      Pass::PreRender();

      Renderer* renderer = GetRenderer();
      m_camera           = m_params.Viewport->GetCamera();
      renderer->SetFramebuffer(m_params.Viewport->m_framebuffer, GraphicBitFields::DepthBits);
      renderer->SetCamera(m_camera, true);

      // Update.
      BillboardPtrArray& gizmoArray = m_params.GizmoArray;
      gizmoArray.erase(std::remove_if(gizmoArray.begin(),
                                      gizmoArray.end(),
                                      [this](EditorBillboardPtr bb) -> bool
                                      {
                                        if (bb == nullptr)
                                        {
                                          return true;
                                        }

                                        bb->LookAt(m_camera, m_params.Viewport->GetBillboardScale());
                                        return false;
                                      }),
                       gizmoArray.end());

      POP_CPU_MARKER();
    }

    void GizmoPass::PostRender()
    {
      PUSH_CPU_MARKER("GizmoPass::PostRender");
      Pass::PostRender();
      POP_CPU_MARKER();
    }

  } // namespace Editor
} // namespace ToolKit