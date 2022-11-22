#include "EditorPass.h"

#include "App.h"
#include "EditorViewport.h"
#include "stdafx.h"

namespace ToolKit
{
  namespace Editor
  {
    void EditorRenderPass::Render()
    {
      RenderPass::Render();
    }

    void EditorRenderPass::PreRender()
    {
      Pass::PreRender();

      App* app = m_params.App;
      m_camera = m_params.Viewport->GetCamera();

      Renderer* renderer = GetRenderer();
      renderer->SetFramebuffer(m_params.Viewport->m_framebuffer, false);
      renderer->SetCameraLens(m_camera);

      // Accumulate Editor entities.
      m_drawList.clear();

      EditorScenePtr scene = app->GetCurrentScene();

      // Generate Selection boundary and Environment component boundary.
      EntityRawPtrArray selecteds;
      scene->GetSelectedEntities(selecteds);

      for (Entity* ntt : selecteds)
      {
        EnvironmentComponentPtr envCom =
            ntt->GetComponent<EnvironmentComponent>();
        if (envCom != nullptr && ntt->GetType() != EntityType::Entity_Sky)
        {
          app->m_perFrameDebugObjects.push_back(CreateBoundingBoxDebugObject(
              envCom->GetBBox(), g_environmentGizmoColor, 1.0f));
        }

        if (app->m_showSelectionBoundary && ntt->IsDrawable())
        {
          app->m_perFrameDebugObjects.push_back(
              CreateBoundingBoxDebugObject(ntt->GetAABB(true)));
        }

        if (app->m_showDirectionalLightShadowFrustum)
        {
          // Directional light shadow map frustum
          if (ntt->GetType() == EntityType::Entity_DirectionalLight)
          {
            EditorDirectionalLight* light =
                static_cast<EditorDirectionalLight*>(ntt);
            if (light->GetCastShadowVal())
            {
              app->m_perFrameDebugObjects.push_back(
                  light->GetDebugShadowFrustum());
            }
          }
        }
      }

      // Per frame objects.
      m_drawList.insert(m_drawList.end(),
                        app->m_perFrameDebugObjects.begin(),
                        app->m_perFrameDebugObjects.end());

      // Billboards.
      std::vector<EditorBillboardBase*> bbs = scene->GetBillboards();
      bbs.push_back(app->m_origin);
      bbs.push_back(app->m_cursor);

      float vpScale = m_params.Viewport->GetBillboardScale();
      for (EditorBillboardBase* bb : bbs)
      {
        bb->LookAt(m_camera, vpScale);
      }

      m_drawList.insert(m_drawList.end(), bbs.begin(), bbs.end());

      // Grid.
      Grid* grid = m_params.Viewport->GetType() == Window::Type::Viewport2d
                       ? app->m_2dGrid
                       : app->m_grid;

      grid->UpdateShaderParams();
      m_drawList.push_back(grid);

      CullDrawList(m_drawList, m_camera);
    }

    void EditorRenderPass::PostRender()
    {
      Pass::PostRender();

      App* app = m_params.App;
      for (Entity* dbgObj : app->m_perFrameDebugObjects)
      {
        SafeDel(dbgObj);
      }
      app->m_perFrameDebugObjects.clear();
    }

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
      PreRender();

      Renderer* renderer = GetRenderer();

      for (EditorBillboardBase* bb : m_params.GizmoArray)
      {
        if (bb->GetBillboardType() ==
            EditorBillboardBase::BillboardType::Rotate)
        {
          Mat4 ts = bb->m_node->GetTransform();
          m_depthMaskSphere->m_node->SetTransform(
              ts, TransformationSpace::TS_WORLD, false);

          renderer->ColorMask(false, false, false, false);
          renderer->Render(m_depthMaskSphere.get(), m_camera);

          renderer->ColorMask(true, true, true, true);
          renderer->Render(bb, m_camera);
        }
        else
        {
          renderer->Render(bb, m_camera);
        }
      }

      PostRender();
    }

    void GizmoPass::PreRender()
    {
      Pass::PreRender();

      Renderer* renderer = GetRenderer();
      m_camera           = m_params.Viewport->GetCamera();
      renderer->SetFramebuffer(m_params.Viewport->m_framebuffer, false);
      renderer->SetCameraLens(m_camera);
      renderer->ClearBuffer(GraphicBitFields::DepthBits);

      for (int i = (int) m_params.GizmoArray.size() - 1; i >= 0; i--)
      {
        if (EditorBillboardBase* bb = m_params.GizmoArray[i])
        {
          bb->LookAt(m_camera, m_params.Viewport->GetBillboardScale());
        }
        else
        {
          m_params.GizmoArray.erase(m_params.GizmoArray.begin() + i);
        }
      }
    }

    void GizmoPass::PostRender()
    {
      Pass::PostRender();
    }

  } // namespace Editor
} // namespace ToolKit
