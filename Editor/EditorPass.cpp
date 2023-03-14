#include "EditorPass.h"

#include "App.h"
#include "DirectionComponent.h"
#include "EditorScene.h"
#include "EditorViewport.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    EditorRenderer::EditorRenderer() { InitRenderer(); }

    EditorRenderer::EditorRenderer(const EditorRenderPassParams& params)
        : EditorRenderer()
    {
      m_params = params;
    }

    EditorRenderer::~EditorRenderer()
    {
      m_lightSystem       = nullptr;
      m_scenePass         = nullptr;
      m_editorPass        = nullptr;
      m_gizmoPass         = nullptr;
      m_tonemapPass       = nullptr;
      m_gammaPass         = nullptr;
      m_fxaaPass          = nullptr;
      m_bloomPass         = nullptr;
      m_ssaoPass          = nullptr;
      m_outlinePass       = nullptr;
      m_singleMatRenderer = nullptr;
    }

    void EditorRenderer::Render(Renderer* renderer)
    {
      PreRender();

      SetLitMode(renderer, m_params.LitMode);

      m_passArray.clear();
      const EngineSettings::PostProcessingSettings& 
          gfx = GetEngineSettings().PostProcessing;
      switch (m_params.LitMode)
      {
      case EditorLitMode::LightComplexity:
      case EditorLitMode::Unlit:
        m_passArray.push_back(m_singleMatRenderer);
        break;
      case EditorLitMode::Game:
        m_params.App->HideGizmos();
        m_scenePass->m_params.Gfx = gfx;
        m_scenePass->Render(renderer);
        break;
      default:
        m_scenePass->m_params.Gfx                        = gfx;
        m_scenePass->m_params.Gfx.GammaCorrectionEnabled = false;
        m_scenePass->m_params.Gfx.TonemappingEnabled     = false;
        m_scenePass->m_params.Gfx.FXAAEnabled            = false;
        m_scenePass->Render(renderer);
        break;
      }

      if (m_params.LitMode != EditorLitMode::Game)
      {
        // Draw scene and apply bloom effect.
        Technique::Render(renderer);
        m_passArray.clear();

        SetLitMode(renderer, EditorLitMode::EditorLit);

        // Draw outlines.
        OutlineSelecteds(renderer);
        m_passArray.clear();

        // Draw editor objects.
        m_passArray.push_back(m_editorPass);
        m_passArray.push_back(m_gizmoPass);

        // Post process.
        m_passArray.push_back(m_tonemapPass);
        if (gfx.FXAAEnabled) 
        {
          m_passArray.push_back(m_fxaaPass);
        }
        m_passArray.push_back(m_gammaPass);

        Technique::Render(renderer);
      }
      else
      {
        m_params.App->ShowGizmos();
      }

      PostRender();
    }

    void EditorRenderer::PreRender()
    {
      App* app = m_params.App;
      m_camera = m_params.Viewport->GetCamera();

      // Adjust scene lights.
      m_lightSystem->m_parentNode->OrphanSelf();
      m_camera->m_node->AddChild(m_lightSystem->m_parentNode);

      // Generate Selection boundary and Environment component boundary.
      EditorScenePtr scene = app->GetCurrentScene();
      m_selecteds.clear();
      scene->GetSelectedEntities(m_selecteds);

      for (Entity* ntt : m_selecteds)
      {
        EnvironmentComponentPtr envCom =
            ntt->GetComponent<EnvironmentComponent>();

        if (envCom != nullptr && ntt->GetType() != EntityType::Entity_Sky)
        {
          app->m_perFrameDebugObjects.push_back(
              CreateBoundingBoxDebugObject(envCom->GetBBox(),
                                           g_environmentGizmoColor,
                                           1.0f));
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
      EntityRawPtrArray editorEntities;
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

      LightRawPtrArray lights = m_params.LitMode == EditorLitMode::EditorLit
                                    ? m_lightSystem->m_lights
                                    : scene->GetLights();

      EditorViewport* viewport =
          static_cast<EditorViewport*>(m_params.Viewport);

      // Editor pass.
      m_editorPass->m_params.Cam              = m_camera;
      m_editorPass->m_params.FrameBuffer      = viewport->m_framebuffer;
      m_editorPass->m_params.Entities         = editorEntities;
      m_editorPass->m_params.ClearFrameBuffer = false;

      // Scene pass.
      m_scenePass->m_params.Cam               = m_camera;
      m_scenePass->m_params.Lights            = lights;
      m_scenePass->m_params.MainFramebuffer   = viewport->m_framebuffer;
      m_scenePass->m_params.Scene             = scene;

      const EngineSettings::PostProcessingSettings& gfx     
                                   = GetEngineSettings().PostProcessing;

      // Bloom pass
      m_bloomPass->m_params.FrameBuffer       = viewport->m_framebuffer;
      m_bloomPass->m_params.intensity         = gfx.BloomIntensity;
      m_bloomPass->m_params.minThreshold      = gfx.BloomThreshold;
      m_bloomPass->m_params.iterationCount    = gfx.BloomIterationCount;

      // Light Complexity pass
      m_singleMatRenderer->m_params.ForwardParams.Cam              = m_camera;
      m_singleMatRenderer->m_params.ForwardParams.Lights           = lights;
      m_singleMatRenderer->m_params.ForwardParams.ClearFrameBuffer = true;

      m_singleMatRenderer->m_params.ForwardParams.Entities =
          scene->GetEntities();

      m_singleMatRenderer->m_params.ForwardParams.FrameBuffer =
          viewport->m_framebuffer;

      m_tonemapPass->m_params.FrameBuffer = viewport->m_framebuffer;
      m_tonemapPass->m_params.Method      = gfx.TonemapperMode;

      // Gamma Pass.
      m_gammaPass->m_params.FrameBuffer   = viewport->m_framebuffer;
      m_gammaPass->m_params.Gamma         = gfx.Gamma;

      // FXAA Pass
      m_fxaaPass->m_params.FrameBuffer    = viewport->m_framebuffer;
      m_fxaaPass->m_params.screen_size    = viewport->m_size;

      // Gizmo Pass.
      m_gizmoPass->m_params.Viewport      = viewport;

      EditorBillboardBase* anchorGizmo    = nullptr;
      if (viewport->GetType() == Window::Type::Viewport2d)
      {
        anchorGizmo = (EditorBillboardBase*) app->m_anchor.get();
      }
      m_gizmoPass->m_params.GizmoArray = {app->m_gizmo, anchorGizmo};
    }

    void EditorRenderer::PostRender()
    {
      App* app = m_params.App;
      for (Entity* dbgObj : app->m_perFrameDebugObjects)
      {
        SafeDel(dbgObj);
      }
      app->m_perFrameDebugObjects.clear();
    }

    void EditorRenderer::SetLitMode(Renderer* renderer, EditorLitMode mode)
    {
      switch (mode)
      {
      case EditorLitMode::EditorLit:
      case EditorLitMode::FullyLit:
      case EditorLitMode::Game:
        renderer->m_renderOnlyLighting = false;
        break;
      case EditorLitMode::LightingOnly:
        renderer->m_renderOnlyLighting = true;
        break;
      case EditorLitMode::Unlit:
        m_singleMatRenderer->m_params.OverrideFragmentShader =
            GetShaderManager()->Create<Shader>(
                ShaderPath("unlitFrag.shader", true));
        renderer->m_renderOnlyLighting = false;
        break;
      case EditorLitMode::LightComplexity:
        m_singleMatRenderer->m_params.OverrideFragmentShader =
            GetShaderManager()->Create<Shader>(
                ShaderPath("lightComplexity.shader", true));
        renderer->m_renderOnlyLighting = false;
        break;
      }
    }

    void EditorRenderer::InitRenderer()
    {
      m_lightSystem   = std::make_shared<ThreePointLightSystem>();

      // Create render mode materials.
      m_unlitOverride = GetMaterialManager()->GetCopyOfUnlitMaterial();
      m_unlitOverride->Init();

      m_scenePass         = std::make_shared<SceneRenderer>();
      m_editorPass        = std::make_shared<ForwardRenderPass>();
      m_gizmoPass         = std::make_shared<GizmoPass>();
      m_tonemapPass       = std::make_shared<TonemapPass>();
      m_gammaPass         = std::make_shared<GammaPass>();
      m_fxaaPass          = std::make_shared<FXAAPass>();
      m_bloomPass         = std::make_shared<BloomPass>();
      m_ssaoPass          = std::make_shared<SSAOPass>();
      m_outlinePass       = std::make_shared<OutlinePass>();
      m_singleMatRenderer = std::make_shared<SingleMatForwardRenderPass>();
    }

    void EditorRenderer::OutlineSelecteds(Renderer* renderer)
    {
      if (m_selecteds.empty())
      {
        return;
      }
      EntityRawPtrArray selecteds = m_selecteds; // Copy

      Viewport* viewport          = m_params.Viewport;
      auto RenderFn =
          [this, viewport, renderer](const EntityRawPtrArray& selection,
                                     const Vec4& color) -> void
      {
        if (selection.empty())
        {
          return;
        }

        // Set parameters of pass
        m_outlinePass->m_params.Camera       = viewport->GetCamera();
        m_outlinePass->m_params.FrameBuffer  = viewport->m_framebuffer;
        m_outlinePass->m_params.OutlineColor = color;
        m_outlinePass->m_params.DrawList     = selection;

        for (Entity* entity : selection)
        {
          // Disable light gizmos
          if (entity->IsLightInstance())
          {
            EnableLightGizmo(static_cast<Light*>(entity), false);
          }

          // Add billboards to draw list
          Entity* billboard =
              m_params.App->GetCurrentScene()->GetBillboardOfEntity(entity);
          if (billboard)
          {
            static_cast<Billboard*>(billboard)->LookAt(
                viewport->GetCamera(),
                viewport->GetBillboardScale());
            m_outlinePass->m_params.DrawList.push_back(billboard);
          }
        }

        m_passArray.clear();
        m_passArray.push_back(m_outlinePass);
        Technique::Render(renderer);

        // Enable light gizmos back
        for (Entity* entity : selection)
        {
          if (entity->IsLightInstance())
          {
            EnableLightGizmo(static_cast<Light*>(entity), true);
          }
        }
      };

      Entity* primary = selecteds.back();

      selecteds.pop_back();
      RenderFn(selecteds, g_selectHighLightSecondaryColor);

      selecteds.clear();
      selecteds.push_back(primary);
      RenderFn(selecteds, g_selectHighLightPrimaryColor);
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
          renderer->Render(m_depthMaskSphere.get(), m_camera);

          renderer->ColorMask(true, true, true, true);
          renderer->Render(bb, m_camera);
        }
        else
        {
          renderer->Render(bb, m_camera);
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

    void GizmoPass::PostRender() { Pass::PostRender(); }

    SingleMatForwardRenderPass::SingleMatForwardRenderPass()
        : ForwardRenderPass()
    {
      m_overrideMat = std::make_shared<Material>();
    }

    SingleMatForwardRenderPass::SingleMatForwardRenderPass(
        const SingleMatForwardRenderPassParams& params)
        : SingleMatForwardRenderPass()
    {
      m_params = params;
    }

    void SingleMatForwardRenderPass::Render()
    {
      EntityRawPtrArray opaqueDrawList;
      EntityRawPtrArray translucentDrawList;
      SeperateTranslucentEntities(m_params.ForwardParams.Entities,
                                  opaqueDrawList,
                                  translucentDrawList);

      Renderer* renderer = GetRenderer();
      for (Entity* ntt : opaqueDrawList)
      {
        LightRawPtrArray lightList =
            GetBestLights(ntt, m_params.ForwardParams.Lights);

        MaterialPtr mat         = ntt->GetRenderMaterial();
        renderer->m_overrideMat = std::make_shared<Material>();
        renderer->m_overrideMat->SetRenderState(mat->GetRenderState());
        renderer->m_overrideMat->m_vertexShader = mat->m_vertexShader;
        renderer->m_overrideMat->m_fragmentShader =
            m_params.OverrideFragmentShader;
        renderer->m_overrideMat->m_diffuseTexture  = mat->m_diffuseTexture;
        renderer->m_overrideMat->m_emissiveTexture = mat->m_emissiveTexture;
        renderer->m_overrideMat->m_emissiveColor   = mat->m_emissiveColor;
        renderer->m_overrideMat->m_cubeMap         = mat->m_cubeMap;
        renderer->m_overrideMat->m_color           = mat->m_color;
        renderer->m_overrideMat->m_alpha           = mat->m_alpha;
        renderer->m_overrideMat->Init();

        renderer->Render(ntt, m_params.ForwardParams.Cam, lightList);
      }

      RenderTranslucent(translucentDrawList,
                        m_camera,
                        m_params.ForwardParams.Lights);
    }

    void SingleMatForwardRenderPass::PreRender()
    {
      ForwardRenderPass::m_params = m_params.ForwardParams;
      ForwardRenderPass::PreRender();
      Renderer* renderer = GetRenderer();

      m_overrideMat->UnInit();
      m_overrideMat->m_fragmentShader = m_params.OverrideFragmentShader;
      m_overrideMat->Init();
    };
  } // namespace Editor
} // namespace ToolKit
