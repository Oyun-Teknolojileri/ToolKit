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
#include "Gizmo.h"

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
      m_billboardPass         = nullptr;
      m_lightSystem           = nullptr;
      m_sceneRenderPath       = nullptr;
      m_mobileSceneRenderPath = nullptr;
      m_uiPass                = nullptr;
      m_editorPass            = nullptr;
      m_gizmoPass             = nullptr;
      m_tonemapPass           = nullptr;
      m_gammaPass             = nullptr;
      m_fxaaPass              = nullptr;
      m_bloomPass             = nullptr;
      m_ssaoPass              = nullptr;
      m_outlinePass           = nullptr;
      m_singleMatRenderer     = nullptr;
    }

    void EditorRenderer::Render(Renderer* renderer)
    {
      PUSH_CPU_MARKER("EditorRenderer::PreRender");

      PreRender();

      POP_CPU_MARKER();
      PUSH_CPU_MARKER("EditorRenderer::Render");

      SetLitMode(renderer, m_params.LitMode);

      m_passArray.clear();
      const EngineSettings::PostProcessingSettings& gfx = GetEngineSettings().PostProcessing;

      SceneRenderPathPtr sceneRenderer                  = nullptr;
      if (m_params.UseMobileRenderPath)
      {
        sceneRenderer = m_mobileSceneRenderPath;
      }
      else
      {
        sceneRenderer = m_sceneRenderPath;
      }

      if (GetRenderSystem()->IsSkipFrame())
      {
        sceneRenderer->m_params.Gfx                        = gfx;
        sceneRenderer->m_params.Gfx.GammaCorrectionEnabled = false;
        sceneRenderer->m_params.Gfx.TonemappingEnabled     = false;
        sceneRenderer->m_params.Gfx.FXAAEnabled            = false;
        sceneRenderer->Render(renderer);

        m_passArray.push_back(m_skipFramePass);
        RenderPath::Render(renderer);

        GetRenderSystem()->DecrementSkipFrame();
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
        sceneRenderer->m_params.Gfx                        = gfx;
        sceneRenderer->m_params.Gfx.GammaCorrectionEnabled = false;
        sceneRenderer->Render(renderer);
        m_passArray.push_back(m_uiPass);
        if (GetRenderSystem()->IsGammaCorrectionNeeded())
        {
          m_passArray.push_back(m_gammaPass);
        }
        RenderPath::Render(renderer);
        m_params.App->ShowGizmos();
        break;
      default:
        sceneRenderer->m_params.Gfx                        = gfx;
        sceneRenderer->m_params.Gfx.GammaCorrectionEnabled = false;
        sceneRenderer->m_params.Gfx.TonemappingEnabled     = false;
        sceneRenderer->m_params.Gfx.FXAAEnabled            = false;
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
        if (gfx.TonemappingEnabled)
        {
          m_passArray.push_back(m_tonemapPass);
        }
        if (gfx.FXAAEnabled)
        {
          if (m_params.Viewport->m_name != g_2dViewport)
          {
            m_passArray.push_back(m_fxaaPass);
          }
        }
        if (GetRenderSystem()->IsGammaCorrectionNeeded())
        {
          m_passArray.push_back(m_gammaPass);
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
      App* app = m_params.App;
      m_camera = m_params.Viewport->GetCamera();

      // Adjust scene lights.
      m_lightSystem->m_parentNode->OrphanSelf();
      m_camera->m_node->AddChild(m_lightSystem->m_parentNode);

      // Generate Selection boundary and Environment component boundary.
      EditorScenePtr scene = app->GetCurrentScene();
      m_selecteds.clear();
      scene->GetSelectedEntities(m_selecteds);

      for (EntityPtr ntt : m_selecteds)
      {
        EnvironmentComponentPtr envCom = ntt->GetComponent<EnvironmentComponent>();

        if (envCom != nullptr && !ntt->IsA<Sky>())
        {
          app->m_perFrameDebugObjects.push_back(
              CreateBoundingBoxDebugObject(envCom->GetBBox(), g_environmentGizmoColor, 1.0f));
        }

        if (app->m_showSelectionBoundary && ntt->IsDrawable())
        {
          app->m_perFrameDebugObjects.push_back(CreateBoundingBoxDebugObject(ntt->GetAABB(true)));
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
      GridPtr grid = m_params.Viewport->GetType() == Window::Type::Viewport2d ? app->m_2dGrid : app->m_grid;

      grid->UpdateShaderParams();
      editorEntities.push_back(grid);

      LightPtrArray lights =
          m_params.LitMode == EditorLitMode::EditorLit ? m_lightSystem->m_lights : scene->GetLights();

      EditorViewport* viewport = static_cast<EditorViewport*>(m_params.Viewport);

      m_renderJobs.clear();
      RenderJobProcessor::CreateRenderJobs(editorEntities, m_renderJobs);
      m_opaque.clear();
      m_translucent.clear();
      RenderJobProcessor::SeperateOpaqueTranslucent(m_renderJobs, m_opaque, m_translucent);

      // Editor pass.
      m_editorPass->m_params.Cam             = m_camera;
      m_editorPass->m_params.FrameBuffer     = viewport->m_framebuffer;
      m_editorPass->m_params.OpaqueJobs      = m_opaque;
      m_editorPass->m_params.TranslucentJobs = m_translucent;
      m_editorPass->m_params.clearBuffer     = GraphicBitFields::None;

      if (m_params.UseMobileRenderPath)
      {
        // Mobile scene pass
        m_mobileSceneRenderPath->m_params.Cam             = m_camera;
        m_mobileSceneRenderPath->m_params.Lights          = lights;
        m_mobileSceneRenderPath->m_params.MainFramebuffer = viewport->m_framebuffer;
        m_mobileSceneRenderPath->m_params.Scene           = scene;
      }
      else
      {
        // Scene pass.
        m_sceneRenderPath->m_params.Cam             = m_camera;
        m_sceneRenderPath->m_params.Lights          = lights;
        m_sceneRenderPath->m_params.MainFramebuffer = viewport->m_framebuffer;
        m_sceneRenderPath->m_params.Scene           = scene;
      }

      // Skip frame pass.
      m_skipFramePass->m_params.FrameBuffer = viewport->m_framebuffer;
      m_skipFramePass->m_material           = m_blackMaterial;

      // UI pass.
      UILayerPtrArray layers;
      m_uiRenderJobs.clear();
      GetUIManager()->GetLayers(viewport->m_viewportId, layers);

      for (const UILayerPtr& layer : layers)
      {
        const EntityPtrArray& uiNtties = layer->m_scene->GetEntities();
        RenderJobProcessor::CreateRenderJobs(uiNtties, m_uiRenderJobs);
      }

      m_uiPass->m_params.OpaqueJobs.clear();
      m_uiPass->m_params.TranslucentJobs.clear();

      RenderJobProcessor::SeperateOpaqueTranslucent(m_uiRenderJobs,
                                                    m_uiPass->m_params.OpaqueJobs,
                                                    m_uiPass->m_params.TranslucentJobs);

      m_uiPass->m_params.Cam                                  = GetUIManager()->GetUICamera();
      m_uiPass->m_params.FrameBuffer                          = viewport->m_framebuffer;
      m_uiPass->m_params.clearBuffer                          = GraphicBitFields::DepthBits;

      const EngineSettings::PostProcessingSettings& gfx       = GetEngineSettings().PostProcessing;

      // Bloom pass
      m_bloomPass->m_params.FrameBuffer                       = viewport->m_framebuffer;
      m_bloomPass->m_params.intensity                         = gfx.BloomIntensity;
      m_bloomPass->m_params.minThreshold                      = gfx.BloomThreshold;
      m_bloomPass->m_params.iterationCount                    = gfx.BloomIterationCount;

      // Light Complexity pass
      m_singleMatRenderer->m_params.ForwardParams.Cam         = m_camera;
      m_singleMatRenderer->m_params.ForwardParams.Lights      = lights;
      m_singleMatRenderer->m_params.ForwardParams.clearBuffer = GraphicBitFields::AllBits;

      m_singleMatRenderer->m_params.ForwardParams.OpaqueJobs  = m_renderJobs;

      m_singleMatRenderer->m_params.ForwardParams.FrameBuffer = viewport->m_framebuffer;

      m_tonemapPass->m_params.FrameBuffer                     = viewport->m_framebuffer;
      m_tonemapPass->m_params.Method                          = gfx.TonemapperMode;

      // Gamma Pass.
      m_gammaPass->m_params.FrameBuffer                       = viewport->m_framebuffer;
      m_gammaPass->m_params.Gamma                             = gfx.Gamma;

      // FXAA Pass
      m_fxaaPass->m_params.FrameBuffer                        = viewport->m_framebuffer;
      m_fxaaPass->m_params.screen_size                        = viewport->m_size;

      // Gizmo Pass.
      m_gizmoPass->m_params.Viewport                          = viewport;

      EditorBillboardPtr anchorGizmo                          = nullptr;
      if (viewport->GetType() == Window::Type::Viewport2d)
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

      m_billboardPass         = MakeNewPtr<BillboardPass>();
      m_sceneRenderPath       = MakeNewPtr<SceneRenderPath>();
      m_mobileSceneRenderPath = MakeNewPtr<MobileSceneRenderPath>();
      m_uiPass                = MakeNewPtr<ForwardRenderPass>();
      m_editorPass            = MakeNewPtr<ForwardRenderPass>();
      m_gizmoPass             = MakeNewPtr<GizmoPass>();
      m_tonemapPass           = MakeNewPtr<TonemapPass>();
      m_gammaPass             = MakeNewPtr<GammaPass>();
      m_fxaaPass              = MakeNewPtr<FXAAPass>();
      m_bloomPass             = MakeNewPtr<BloomPass>();
      m_ssaoPass              = MakeNewPtr<SSAOPass>();
      m_outlinePass           = MakeNewPtr<OutlinePass>();
      m_singleMatRenderer     = MakeNewPtr<SingleMatForwardRenderPass>();
      m_skipFramePass         = MakeNewPtr<FullQuadPass>();
    }

    void EditorRenderer::OutlineSelecteds(Renderer* renderer)
    {
      if (m_selecteds.empty())
      {
        return;
      }
      EntityPtrArray selecteds = m_selecteds; // Copy

      Viewport* viewport       = m_params.Viewport;
      auto RenderFn            = [this, viewport, renderer](const EntityPtrArray& selection, const Vec4& color) -> void
      {
        if (selection.empty())
        {
          return;
        }

        RenderJobArray renderJobs;
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
            static_cast<Billboard*>(billboard.get())->LookAt(viewport->GetCamera(), viewport->GetBillboardScale());

            RenderJobArray jobs;
            RenderJobProcessor::CreateRenderJobs({billboard}, jobs);
            renderJobs.insert(renderJobs.end(), jobs.begin(), jobs.end());
          }
        }

        RenderJobProcessor::CreateRenderJobs(selection, renderJobs, true);

        // Set parameters of pass
        m_outlinePass->m_params.Camera       = viewport->GetCamera();
        m_outlinePass->m_params.FrameBuffer  = viewport->m_framebuffer;
        m_outlinePass->m_params.OutlineColor = color;
        m_outlinePass->m_params.RenderJobs   = renderJobs;

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
