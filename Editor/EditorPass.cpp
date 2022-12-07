#include "EditorPass.h"

#include "App.h"
#include "DirectionComponent.h"
#include "EditorScene.h"
#include "EditorViewport.h"
#include "stdafx.h"

namespace ToolKit
{
  namespace Editor
  {

    EditorRenderer::EditorRenderer()
    {
      m_editorScene = std::make_shared<EditorScene>();
      InitRenderer();
    }

    EditorRenderer::EditorRenderer(const EditorRenderPassParams& params)
        : EditorRenderer()
    {
      m_params = params;
    }

    EditorRenderer::~EditorRenderer()
    {
      for (Light* light : m_editorLights)
      {
        SafeDel(light);
      }
      m_editorLights.clear();
      SafeDel(m_lightNode);

      // Prevent entities to be destroyed.
      m_editorScene->AccessEntityArray().clear();
    }

    void EditorRenderer::Render()
    {
      PreRender();

      SetLitMode(m_params.LitMode);

      m_scenePass.Render();

      SetLitMode(EditorLitMode::EditorLit);

      m_editorPass.Render();

      m_gizmoPass.Render();

      m_gammaPass.Render();

      PostRender();
    }

    void EditorRenderer::PreRender()
    {
      App* app = m_params.App;
      m_camera = m_params.Viewport->GetCamera();

      // Adjust scene lights.
      m_lightNode->OrphanSelf();
      m_camera->m_node->AddChild(m_lightNode);

      Renderer* renderer       = GetRenderer();
      m_overrideDiffuseTexture = renderer->m_overrideDiffuseTexture;

      // Construct EditorScene
      EntityRawPtrArray editorEntities;
      EntityRawPtrArray selecteds;

      // Generate Selection boundary and Environment component boundary.
      EditorScenePtr scene = app->GetCurrentScene();
      scene->GetSelectedEntities(selecteds);

      for (Entity* ntt : selecteds)
      {
        EnvironmentComponentPtr envCom =
            ntt->GetComponent<EnvironmentComponent>();

        if (envCom != nullptr && ntt->GetType() != EntityType::Entity_Sky)
        {
          editorEntities.push_back(CreateBoundingBoxDebugObject(
              envCom->GetBBox(), g_environmentGizmoColor, 1.0f));
        }

        if (app->m_showSelectionBoundary && ntt->IsDrawable())
        {
          editorEntities.push_back(
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
              editorEntities.push_back(light->GetDebugShadowFrustum());
            }
          }
        }
      }

      // Per frame objects.
      editorEntities.insert(editorEntities.end(),
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

      editorEntities.insert(editorEntities.end(), bbs.begin(), bbs.end());

      // Grid.
      Grid* grid = m_params.Viewport->GetType() == Window::Type::Viewport2d
                       ? app->m_2dGrid
                       : app->m_grid;

      grid->UpdateShaderParams();
      editorEntities.push_back(grid);

      // Nothing lit, so no lights necessary.
      m_editorScene->AccessEntityArray() = editorEntities;

      LightRawPtrArray lights = m_params.LitMode == EditorLitMode::FullyLit
                                    ? scene->GetLights()
                                    : m_editorLights;

      EditorViewport* viewport =
          static_cast<EditorViewport*>(m_params.Viewport);

      // Editor pass.
      m_editorPass.m_params.Cam              = viewport->GetCamera();
      m_editorPass.m_params.FrameBuffer      = viewport->m_framebuffer;
      m_editorPass.m_params.Scene            = m_editorScene;
      m_editorPass.m_params.ClearFrameBuffer = false;

      // Shadow pass.
      m_scenePass.m_params.shadowPassParams.Entities = scene->GetEntities();
      m_scenePass.m_params.shadowPassParams.Lights   = lights;

      // Scene Pass.
      m_scenePass.m_params.renderPassParams.Scene = app->GetCurrentScene();
      m_scenePass.m_params.renderPassParams.LightOverride = lights;
      m_scenePass.m_params.renderPassParams.Cam           = m_camera;
      m_scenePass.m_params.renderPassParams.FrameBuffer =
          viewport->m_framebuffer;

      // Gamma Pass.
      m_gammaPass.m_params.FrameBuffer = viewport->m_framebuffer;
      // TODO: Read it from engine settings.
      m_gammaPass.m_params.Gamma       = 2.2f;

      // Gizmo Pass.
      m_gizmoPass.m_params.Viewport = viewport;

      EditorBillboardBase* anchorGizmo = nullptr;
      if (viewport->GetType() == Window::Type::Viewport2d)
      {
        anchorGizmo = (EditorBillboardBase*) app->m_anchor.get();
      }
      m_gizmoPass.m_params.GizmoArray = {app->m_gizmo, anchorGizmo};
    }

    void EditorRenderer::PostRender()
    {
      GetRenderer()->m_overrideDiffuseTexture = m_overrideDiffuseTexture;

      App* app = m_params.App;
      for (Entity* dbgObj : app->m_perFrameDebugObjects)
      {
        SafeDel(dbgObj);
      }
      app->m_perFrameDebugObjects.clear();
      m_lightNode->OrphanSelf();
    }

    void EditorRenderer::SetLitMode(EditorLitMode mode)
    {
      Renderer* renderer = GetRenderer();
      switch (mode)
      {
      case EditorLitMode::EditorLit:
        renderer->m_overrideMat = nullptr;
        break;
      case EditorLitMode::Unlit:
        renderer->m_overrideMat = m_unlitOverride;
        break;
      case EditorLitMode::FullyLit:
        renderer->m_overrideMat = nullptr;
        break;
      case EditorLitMode::LightComplexity:
        renderer->m_overrideMat = m_lightComplexityOverride;
        break;
      case EditorLitMode::LightingOnly:
        renderer->m_overrideMat        = m_lightingOnlyOverride;
        renderer->m_renderOnlyLighting = false;
        break;
      default:
        break;
      }
    }
    void EditorRenderer::CreateEditorLights(LightRawPtrArray& list,
                                            Node** parentNode)
    {
      *parentNode = new Node();

      float intensity         = 1.5f;
      DirectionalLight* light = new DirectionalLight();
      light->SetColorVal(Vec3(0.55f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(-20.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(-20.0f));
      light->SetCastShadowVal(false);
      (*parentNode)->AddChild(light->m_node);
      list.push_back(light);

      light = new DirectionalLight();
      light->SetColorVal(Vec3(0.15f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(90.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(-45.0f));
      light->SetCastShadowVal(false);
      (*parentNode)->AddChild(light->m_node);
      list.push_back(light);

      light = new DirectionalLight();
      light->SetColorVal(Vec3(0.1f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(120.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(60.0f));
      light->SetCastShadowVal(false);
      (*parentNode)->AddChild(light->m_node);
      list.push_back(light);
    }

    void EditorRenderer::InitRenderer()
    {
      CreateEditorLights(m_editorLights, &m_lightNode);

      // Create render mode materials.
      m_lightComplexityOverride = std::make_shared<Material>();
      m_lightComplexityOverride->m_fragmentShader =
          GetShaderManager()->Create<Shader>(
              ShaderPath("lightComplexity.shader", true));
      m_lightComplexityOverride->Init();

      m_lightingOnlyOverride = GetMaterialManager()->GetCopyOfSolidMaterial();
      m_lightingOnlyOverride->Init();

      m_unlitOverride = GetMaterialManager()->GetCopyOfUnlitMaterial();
      m_unlitOverride->Init();
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
