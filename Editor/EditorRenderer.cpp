/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "EditorRenderer.h"

#include "App.h"
#include "EditorScene.h"
#include "EditorViewport.h"
#include "EditorViewport2d.h"
#include "Gizmo.h"
#include "LightMeshGenerator.h"
#include "Util.h"

#include <BVH.h>
#include <Camera.h>
#include <DirectionComponent.h>
#include <EnvironmentComponent.h>
#include <GradientSky.h>
#include <Material.h>
#include <MaterialComponent.h>
#include <TKProfiler.h>
#include <UIManager.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {

    EditorRenderer::EditorRenderer() { InitRenderer(); }

    EditorRenderer::EditorRenderer(const EditorRenderParams& params) : EditorRenderer() { m_params = params; }

    EditorRenderer::~EditorRenderer()
    {
      m_billboardPass        = nullptr;
      m_lightSystem          = nullptr;
      m_sceneRenderPath      = nullptr;
      m_uiPass               = nullptr;
      m_editorPass           = nullptr;
      m_gizmoPass            = nullptr;
      m_outlinePass          = nullptr;
      m_singleMatRenderer    = nullptr;
      m_gammaTonemapFxaaPass = nullptr;
    }

    void EditorRenderer::Render(Renderer* renderer)
    {
      PUSH_CPU_MARKER("EditorRenderer::PreRender");

      PreRender();

      POP_CPU_MARKER();
      PUSH_CPU_MARKER("EditorRenderer::Render");

      SetLitMode(renderer, m_params.LitMode);

      m_passArray.clear();

      SceneRenderPathPtr sceneRenderer = m_sceneRenderPath;
      if (GetRenderSystem()->IsSkipFrame())
      {
        sceneRenderer->Render(renderer);

        m_passArray.push_back(m_skipFramePass);
        RenderPath::Render(renderer);

        PostRender();
        return;
      }

      switch (m_params.LitMode)
      {
      case EditorLitMode::LightComplexity:
      case EditorLitMode::Unlit:
        m_passArray.push_back(m_singleMatRenderer);
        break;
      case EditorLitMode::Game:
        m_params.App->HideGizmos();
        sceneRenderer->Render(renderer);
        m_passArray.push_back(m_uiPass);
        if (m_gammaTonemapFxaaPass->m_params.enableFxaa || m_gammaTonemapFxaaPass->m_params.enableGammaCorrection ||
            m_gammaTonemapFxaaPass->m_params.enableTonemapping)
        {
          m_passArray.push_back(m_gammaTonemapFxaaPass);
        }
        RenderPath::Render(renderer);
        m_params.App->ShowGizmos();
        break;
      default:
        sceneRenderer->Render(renderer);
        break;
      }

      POP_CPU_MARKER();
      PUSH_CPU_MARKER("EditorRender Editor & PostProcess Render");

      if (m_params.LitMode != EditorLitMode::Game)
      {
        // Draw scene and apply bloom effect.
        RenderPath::Render(renderer);
        m_passArray.clear();

        SetLitMode(renderer, EditorLitMode::EditorLit);

        // Draw outlines.
        OutlineSelecteds(renderer);
        m_passArray.clear();

        // Draw editor objects.
        m_passArray.push_back(m_editorPass);
        // Clears depth buffer to draw remaining entities always on top.
        m_passArray.push_back(m_gizmoPass);
        // Scene meshs can't block editor billboards. Desired for this case.
        m_passArray.push_back(m_billboardPass);

        // Post process.
        if (m_gammaTonemapFxaaPass->m_params.enableFxaa || m_gammaTonemapFxaaPass->m_params.enableGammaCorrection ||
            m_gammaTonemapFxaaPass->m_params.enableTonemapping)
        {
          m_passArray.push_back(m_gammaTonemapFxaaPass);
        }

        RenderPath::Render(renderer);
      }

      POP_CPU_MARKER();

      PUSH_CPU_MARKER("EditorRender::PostRender");
      PostRender();
      POP_CPU_MARKER();
    }

    void EditorRenderer::PreRender()
    {
      App* app                                          = m_params.App;
      m_camera                                          = m_params.Viewport->GetCamera();

      const EngineSettings::PostProcessingSettings& gfx = GetEngineSettings().PostProcessing;

      // Adjust scene lights.
      m_lightSystem->m_parentNode->OrphanSelf();
      m_camera->m_node->AddChild(m_lightSystem->m_parentNode);

      EditorScenePtr scene = app->GetCurrentScene();

      LightPtrArray lights;
      bool useSceneLights = true;
      if (m_params.LitMode == EditorLitMode::EditorLit)
      {
        lights         = m_lightSystem->m_lights;
        useSceneLights = false;
      }
      else
      {
        lights = scene->GetLights();
      }

      EditorViewport* viewport                    = static_cast<EditorViewport*>(m_params.Viewport);

      // Scene renderer will render the given scene independent of editor.
      // Editor objects will be drawn on top of scene.

      // Scene pass.
      m_sceneRenderPath->m_params.Gfx             = gfx;
      m_sceneRenderPath->m_params.Cam             = m_camera;
      m_sceneRenderPath->m_params.Lights          = lights;
      m_sceneRenderPath->m_params.MainFramebuffer = viewport->m_framebuffer;
      m_sceneRenderPath->m_params.Scene           = scene;

      if (app->m_showSceneBoundary)
      {
        app->m_perFrameDebugObjects.push_back(CreateBoundingBoxDebugObject(scene->GetSceneBoundary()));
      }

      if (app->m_showBVHNodes)
      {
        scene->m_bvh->GetDebugBVHBoxes(app->m_perFrameDebugObjects);
      }

      if (app->m_showPickingDebug)
      {
        if (app->m_dbgArrow)
        {
          app->m_perFrameDebugObjects.push_back(app->m_dbgArrow);
        }

        if (app->m_dbgFrustum)
        {
          app->m_perFrameDebugObjects.push_back(app->m_dbgFrustum);
        }
      }

      // Generate Selection boundary and Environment component boundary.
      m_selecteds.clear();
      scene->GetSelectedEntities(m_selecteds);

      // Construct gizmo objects.
      for (EntityPtr ntt : m_selecteds)
      {
        EnvironmentComponentPtr envCom = ntt->GetComponent<EnvironmentComponent>();

        if (envCom != nullptr && !ntt->IsA<Sky>())
        {
          app->m_perFrameDebugObjects.push_back(
              CreateBoundingBoxDebugObject(envCom->GetBoundingBox(), g_environmentGizmoColor, 1.0f));
        }

        if (app->m_showSelectionBoundary && ntt->IsDrawable())
        {
          app->m_perFrameDebugObjects.push_back(CreateBoundingBoxDebugObject(ntt->GetBoundingBox(true)));
        }

        if (app->m_showDirectionalLightShadowFrustum)
        {
          // Directional light shadow map frustum
          if (ntt->IsA<DirectionalLight>())
          {
            EditorDirectionalLight* light = static_cast<EditorDirectionalLight*>(ntt.get());
            if (light->GetCastShadowVal())
            {
              app->m_perFrameDebugObjects.push_back(light->GetDebugShadowFrustum());
              app->m_perFrameDebugObjects.push_back(
                  CreateDebugFrustum(app->GetViewport(g_3dViewport)->GetCamera(), Vec3(0.6f, 0.2f, 0.8f), 1.5f));
            }
          }
        }
      }

      // Per frame objects.
      EntityPtrArray editorEntities;
      editorEntities.insert(editorEntities.end(),
                            app->m_perFrameDebugObjects.begin(),
                            app->m_perFrameDebugObjects.end());

      // Billboard pass.
      m_billboardPass->m_params.Billboards = scene->GetBillboards();
      m_billboardPass->m_params.Billboards.push_back(app->m_origin);
      m_billboardPass->m_params.Billboards.push_back(app->m_cursor);
      m_billboardPass->m_params.Viewport = m_params.Viewport;

      // Grid.
      GridPtr grid                       = m_params.Viewport->IsA<EditorViewport2d>() ? app->m_2dGrid : app->m_grid;

      grid->UpdateShaderParams();
      editorEntities.push_back(grid);

      for (LightPtr& light : scene->GetLights())
      {
        if (light->GetLightType() == Light::LightType::Directional)
        {
          if (Cast<EditorDirectionalLight>(light)->GizmoActive())
          {
            editorEntities.push_back(light);
          }
        }
        else if (light->GetLightType() == Light::LightType::Spot)
        {
          if (Cast<EditorSpotLight>(light)->GizmoActive())
          {
            editorEntities.push_back(light);
          }
        }
        else // if (light->GetLightType() == Light::LightType::Point)
        {
          if (Cast<EditorPointLight>(light)->GizmoActive())
          {
            editorEntities.push_back(light);
          }
        }
      }

      // Editor pass.
      m_renderData.jobs.clear();
      RenderJobProcessor::CreateRenderJobs(editorEntities, m_renderData.jobs);
      RenderJobProcessor::SeperateRenderData(m_renderData, true);

      m_editorPass->m_params.renderData     = &m_renderData;
      m_editorPass->m_params.Cam            = m_camera;
      m_editorPass->m_params.FrameBuffer    = viewport->m_framebuffer;
      m_editorPass->m_params.clearBuffer    = GraphicBitFields::None;

      // Skip frame pass.
      m_skipFramePass->m_params.FrameBuffer = viewport->m_framebuffer;
      m_skipFramePass->m_material           = m_blackMaterial;

      // UI pass.
      UILayerPtrArray layers;
      m_uiRenderData.jobs.clear();
      GetUIManager()->GetLayers(viewport->m_viewportId, layers);

      for (const UILayerPtr& layer : layers)
      {
        const EntityPtrArray& uiNtties = layer->m_scene->GetEntities();
        RenderJobProcessor::CreateRenderJobs(uiNtties, m_uiRenderData.jobs);
      }

      RenderJobProcessor::SeperateRenderData(m_uiRenderData, true);

      m_uiPass->m_params.renderData                           = &m_uiRenderData;
      m_uiPass->m_params.Cam                                  = GetUIManager()->GetUICamera();
      m_uiPass->m_params.FrameBuffer                          = viewport->m_framebuffer;
      m_uiPass->m_params.clearBuffer                          = GraphicBitFields::DepthBits;

      // Post process pass

      m_gammaTonemapFxaaPass->m_params.frameBuffer            = viewport->m_framebuffer;
      m_gammaTonemapFxaaPass->m_params.enableGammaCorrection  = GetRenderSystem()->IsGammaCorrectionNeeded();
      m_gammaTonemapFxaaPass->m_params.enableFxaa             = gfx.FXAAEnabled;
      m_gammaTonemapFxaaPass->m_params.enableTonemapping      = gfx.TonemappingEnabled;
      m_gammaTonemapFxaaPass->m_params.gamma                  = gfx.Gamma;
      m_gammaTonemapFxaaPass->m_params.screenSize             = viewport->m_size;
      m_gammaTonemapFxaaPass->m_params.tonemapMethod          = gfx.TonemapperMode;

      // Light Complexity pass
      m_singleMatRenderer->m_params.ForwardParams.renderData  = &m_renderData;
      m_singleMatRenderer->m_params.ForwardParams.Cam         = m_camera;
      m_singleMatRenderer->m_params.ForwardParams.Lights      = lights;
      m_singleMatRenderer->m_params.ForwardParams.clearBuffer = GraphicBitFields::AllBits;

      m_singleMatRenderer->m_params.ForwardParams.FrameBuffer = viewport->m_framebuffer;

      // Gizmo Pass.
      m_gizmoPass->m_params.Viewport                          = viewport;

      EditorBillboardPtr anchorGizmo                          = nullptr;
      if (viewport->IsA<EditorViewport2d>())
      {
        anchorGizmo = app->m_anchor;
      }
      m_gizmoPass->m_params.GizmoArray = {app->m_gizmo, anchorGizmo};
    }

    void EditorRenderer::PostRender() { m_params.App->m_perFrameDebugObjects.clear(); }

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
            GetShaderManager()->Create<Shader>(ShaderPath("unlitFrag.shader", true));
        renderer->m_renderOnlyLighting = false;
        break;
      case EditorLitMode::LightComplexity:
        m_singleMatRenderer->m_params.OverrideFragmentShader =
            GetShaderManager()->Create<Shader>(ShaderPath("lightComplexity.shader", true));
        renderer->m_renderOnlyLighting = false;
        break;
      }
    }

    void EditorRenderer::InitRenderer()
    {
      m_lightSystem   = MakeNewPtr<ThreePointLightSystem>();

      // Create render mode materials.
      m_unlitOverride = GetMaterialManager()->GetCopyOfUnlitMaterial();
      m_blackMaterial = GetMaterialManager()->GetCopyOfUnlitMaterial();
      m_unlitOverride->Init();
      m_blackMaterial->Init();

      m_billboardPass        = MakeNewPtr<BillboardPass>();
      m_sceneRenderPath      = MakeNewPtr<ForwardSceneRenderPath>();
      m_uiPass               = MakeNewPtr<ForwardRenderPass>();
      m_editorPass           = MakeNewPtr<ForwardRenderPass>();
      m_gizmoPass            = MakeNewPtr<GizmoPass>();
      m_outlinePass          = MakeNewPtr<OutlinePass>();
      m_singleMatRenderer    = MakeNewPtr<SingleMatForwardRenderPass>();
      m_skipFramePass        = MakeNewPtr<FullQuadPass>();
      m_gammaTonemapFxaaPass = MakeNewPtr<GammaTonemapFxaaPass>();
    }

    void EditorRenderer::OutlineSelecteds(Renderer* renderer)
    {
      if (m_selecteds.empty())
      {
        return;
      }
      EntityPtrArray selecteds = m_selecteds; // Copy

      Viewport* viewport       = m_params.Viewport;
      CameraPtr viewportCamera = viewport->GetCamera();
      auto RenderFn            = [this, viewport, renderer, &viewportCamera](const EntityPtrArray& selection,
                                                                  const Vec4& color) -> void
      {
        if (selection.empty())
        {
          return;
        }

        RenderJobArray renderJobs;
        RenderJobArray billboardJobs;
        for (EntityPtr entity : selection)
        {
          // Disable light gizmos
          if (Light* light = entity->As<Light>())
          {
            EnableLightGizmo(light, false);
          }

          // Add billboards to draw list
          EntityPtr billboard = m_params.App->GetCurrentScene()->GetBillboard(entity);

          if (billboard)
          {
            static_cast<Billboard*>(billboard.get())->LookAt(viewportCamera, viewport->GetBillboardScale());

            RenderJobProcessor::CreateRenderJobs({billboard}, billboardJobs);
          }
        }

        RenderJobProcessor::CreateRenderJobs(selection, renderJobs, true);
        renderJobs.insert(renderJobs.end(), billboardJobs.begin(), billboardJobs.end());

        RenderJobProcessor::CullRenderJobs(renderJobs, viewportCamera, m_unCulledRenderJobs);

        // Set parameters of pass
        m_outlinePass->m_params.Camera       = viewportCamera;
        m_outlinePass->m_params.FrameBuffer  = viewport->m_framebuffer;
        m_outlinePass->m_params.OutlineColor = color;
        m_outlinePass->m_params.RenderJobs   = &m_unCulledRenderJobs;

        m_passArray.clear();
        m_passArray.push_back(m_outlinePass);
        RenderPath::Render(renderer);

        // Enable light gizmos back
        for (EntityPtr entity : selection)
        {
          if (Light* light = entity->As<Light>())
          {
            EnableLightGizmo(light, true);
          }
        }
      };

      EntityPtr primary = selecteds.back();

      selecteds.pop_back();
      RenderFn(selecteds, g_selectHighLightSecondaryColor);

      selecteds.clear();
      selecteds.push_back(primary);
      RenderFn(selecteds, g_selectHighLightPrimaryColor);
    }

  } // namespace Editor
} // namespace ToolKit
