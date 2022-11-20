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

      // Accumulate Editor entities.
      m_drawList.clear();
      App* app = m_params.App;

      EditorScenePtr scene = app->GetCurrentScene();
      
      // Generate Environment component boxes.
      EntityRawPtrArray selecteds;
      scene->GetSelectedEntities(selecteds);

      for (Entity* ntt : selecteds)
      {
        EnvironmentComponentPtr envCom =
            ntt->GetComponent<EnvironmentComponent>();
        if (envCom != nullptr && ntt->GetType() != EntityType::Entity_Sky)
        {
          app->m_perFrameDebugObjects.push_back(CreateBoundingBoxDebugObject(
              *envCom->GetBBox(), g_environmentGizmoColor, 1.0f));
        }
      }

      // Per frame objects.
      m_drawList.insert(m_drawList.end(),
                        app->m_perFrameDebugObjects.begin(),
                        app->m_perFrameDebugObjects.end());

      // Billboards.
      Renderer* renderer = GetRenderer();
      renderer->SetFramebuffer(m_params.FrameBuffer, true);

      Camera* cam   = m_params.Viewport->GetCamera();
      float vpScale = m_params.Viewport->GetBillboardScale();

      renderer->SetCameraLens(cam);

      std::vector<EditorBillboardBase*> bbs = scene->GetBillboards();
      bbs.push_back(app->m_origin);
      bbs.push_back(app->m_cursor);

      for (EditorBillboardBase* bb : bbs)
      {
        bb->LookAt(cam, vpScale);
      }

      m_drawList.insert(m_drawList.end(), bbs.begin(), bbs.end());

      // Grid.
      if (m_params.Viewport->GetType() == Window::Type::Viewport2d)
      {
        m_drawList.push_back(app->m_2dGrid);
      }
      else
      {
        m_drawList.push_back(app->m_grid);
      }

      CullDrawList(m_drawList, cam);
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

  } // namespace Editor
} // namespace ToolKit
