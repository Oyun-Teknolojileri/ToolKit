#include "EditorPass.h"

#include "App.h"
#include "DirectionComponent.h"
#include "EditorViewport.h"
#include "stdafx.h"

namespace ToolKit
{
  namespace Editor
  {

    EditorRenderPass::EditorRenderPass()
    {
      InitPass();
    }

    EditorRenderPass::EditorRenderPass(const EditorRenderPassParams& params)
        : EditorRenderPass()
    {
      m_params = params;
    }

    EditorRenderPass::~EditorRenderPass()
    {
      for (Light* light : m_editorLights)
      {
        SafeDel(light);
      }
      m_editorLights.clear();
      SafeDel(m_lightNode);
    }

    void EditorRenderPass::Render()
    {
      RenderPass::Render();
    }

    void EditorRenderPass::PreRender()
    {
      Renderer* renderer = GetRenderer();
      Pass::PreRender();
      m_overrideDiffuseTexture = renderer->m_overrideDiffuseTexture;

      App* app = m_params.App;
      m_camera = m_params.Viewport->GetCamera();

      renderer->SetFramebuffer(m_params.Viewport->m_framebuffer);
      renderer->SetCameraLens(m_camera);

      // Accumulate Editor entities.
      m_drawList.clear();

      EditorScenePtr scene              = app->GetCurrentScene();
      const EntityRawPtrArray& entities = scene->GetEntities();
      m_drawList.insert(m_drawList.end(), entities.begin(), entities.end());

      // Adjust scene lights.
      m_lightNode->OrphanSelf();
      m_camera->m_node->AddChild(m_lightNode);

      SetLitMode(m_params.LitMode);

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
      GetRenderer()->m_overrideDiffuseTexture = m_overrideDiffuseTexture;

      App* app = m_params.App;
      for (Entity* dbgObj : app->m_perFrameDebugObjects)
      {
        SafeDel(dbgObj);
      }
      app->m_perFrameDebugObjects.clear();
      m_lightNode->OrphanSelf();
    }

    void EditorRenderPass::SetLitMode(EditorLitMode mode)
    {
      EditorScenePtr scene = m_params.App->GetCurrentScene();
      if (mode != EditorLitMode::EditorLit)
      {
        m_contributingLights = scene->GetLights();
      }

      Renderer* renderer = GetRenderer();

      switch (mode)
      {
      case EditorLitMode::EditorLit:
        m_contributingLights    = m_editorLights;
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
        renderer->m_overrideMat = m_lightingOnlyOverride;
        break;
      default:
        break;
      }
    }

    void EditorRenderPass::InitPass()
    {
      // Create editor lights.
      m_lightNode = new Node();

      float intensity         = 1.5f;
      DirectionalLight* light = new DirectionalLight();
      light->SetColorVal(Vec3(0.55f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(-20.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(-20.0f));
      light->SetCastShadowVal(false);
      m_lightNode->AddChild(light->m_node);
      m_editorLights.push_back(light);

      light = new DirectionalLight();
      light->SetColorVal(Vec3(0.15f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(90.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(-45.0f));
      light->SetCastShadowVal(false);
      m_lightNode->AddChild(light->m_node);
      m_editorLights.push_back(light);

      light = new DirectionalLight();
      light->SetColorVal(Vec3(0.1f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(120.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(60.0f));
      light->SetCastShadowVal(false);
      m_lightNode->AddChild(light->m_node);
      m_editorLights.push_back(light);

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
