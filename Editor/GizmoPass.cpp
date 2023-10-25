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
          m_depthMaskSphere->m_node->SetTransform(ts, TransformationSpace::TS_WORLD, false);

          renderer->ColorMask(false, false, false, false);

          static thread_local RenderJobArray jobs;
          jobs.clear();
          RenderJobProcessor::CreateRenderJobs({m_depthMaskSphere}, jobs);
          renderer->Render(jobs, m_camera);

          renderer->ColorMask(true, true, true, true);

          jobs.clear();
          RenderJobProcessor::CreateRenderJobs({billboard}, jobs);
          renderer->Render(jobs, m_camera);
        }
        else
        {
          static thread_local RenderJobArray jobs;
          jobs.clear();
          RenderJobProcessor::CreateRenderJobs({billboard}, jobs);
          renderer->Render(jobs, m_camera);
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
      renderer->SetFramebuffer(m_params.Viewport->m_framebuffer, false);
      renderer->SetCameraLens(m_camera);
      renderer->ClearBuffer(GraphicBitFields::DepthBits, Vec4(1.0f));

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