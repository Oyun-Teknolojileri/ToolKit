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

#include "EditorRenderer.h"

#include "App.h"
#include "DirectionComponent.h"
#include "EditorScene.h"
#include "EditorViewport.h"
#include "EnvironmentComponent.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    EditorRenderer::EditorRenderer() { InitRenderer(); }

    EditorRenderer::EditorRenderer(const EditorRenderParams& params) : EditorRenderer() { m_params = params; }

    EditorRenderer::~EditorRenderer()
    {
      m_billboardPass     = nullptr;
      m_lightSystem       = nullptr;
      m_scenePass         = nullptr;
      m_uiPass            = nullptr;
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
      const EngineSettings::PostProcessingSettings& gfx = GetEngineSettings().PostProcessing;
      switch (m_params.LitMode)
      {
      case EditorLitMode::LightComplexity:
      case EditorLitMode::Unlit:
        m_passArray.push_back(m_singleMatRenderer);
        break;
      case EditorLitMode::Game:
        m_params.App->HideGizmos();
        m_scenePass->m_params.Gfx                        = gfx;
        m_scenePass->m_params.Gfx.GammaCorrectionEnabled = false;
        m_scenePass->Render(renderer);
        m_passArray.push_back(m_uiPass);
        m_passArray.push_back(m_gammaPass);
        Technique::Render(renderer);
        m_params.App->ShowGizmos();
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
        // Clears depth buffer to draw remaining entities always on top.
        m_passArray.push_back(m_gizmoPass);
        // Scene meshs can't block editor billboards. Desired for this case.
        m_passArray.push_back(m_billboardPass);

        // Post process.
        m_passArray.push_back(m_tonemapPass);
        if (gfx.FXAAEnabled)
        {
          if (m_params.Viewport->m_name != g_2dViewport)
          {
            m_passArray.push_back(m_fxaaPass);
          }
        }
        m_passArray.push_back(m_gammaPass);

        Technique::Render(renderer);
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
        EnvironmentComponentPtr envCom = ntt->GetComponent<EnvironmentComponent>();

        if (envCom != nullptr && ntt->GetType() != EntityType::Entity_Sky)
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
          if (ntt->GetType() == EntityType::Entity_DirectionalLight)
          {
            EditorDirectionalLight* light = static_cast<EditorDirectionalLight*>(ntt);
            if (light->GetCastShadowVal())
            {
              app->m_perFrameDebugObjects.push_back(light->GetDebugShadowFrustum());
            }
          }
        }
      }

      // Per frame objects.
      EntityRawPtrArray editorEntities;
      editorEntities.insert(editorEntities.end(),
                            app->m_perFrameDebugObjects.begin(),
                            app->m_perFrameDebugObjects.end());

      // Billboard pass.
      m_billboardPass->m_params.Billboards = scene->GetBillboards();
      m_billboardPass->m_params.Billboards.push_back(app->m_origin);
      m_billboardPass->m_params.Billboards.push_back(app->m_cursor);
      m_billboardPass->m_params.Viewport = m_params.Viewport;

      // Grid.
      Grid* grid = m_params.Viewport->GetType() == Window::Type::Viewport2d ? app->m_2dGrid : app->m_grid;

      grid->UpdateShaderParams();
      editorEntities.push_back(grid);

      LightRawPtrArray lights =
          m_params.LitMode == EditorLitMode::EditorLit ? m_lightSystem->m_lights : scene->GetLights();

      EditorViewport* viewport = static_cast<EditorViewport*>(m_params.Viewport);

      RenderJobArray renderJobs;
      RenderJobArray opaque;
      RenderJobArray translucent;

      RenderJobProcessor::CreateRenderJobs(editorEntities, renderJobs);
      RenderJobProcessor::SeperateOpaqueTranslucent(renderJobs, opaque, translucent);

      // Editor pass.
      m_editorPass->m_params.Cam              = m_camera;
      m_editorPass->m_params.FrameBuffer      = viewport->m_framebuffer;
      m_editorPass->m_params.OpaqueJobs       = opaque;
      m_editorPass->m_params.TranslucentJobs  = translucent;
      m_editorPass->m_params.ClearFrameBuffer = false;

      // Scene pass.
      m_scenePass->m_params.Cam               = m_camera;
      m_scenePass->m_params.Lights            = lights;
      m_scenePass->m_params.MainFramebuffer   = viewport->m_framebuffer;
      m_scenePass->m_params.Scene             = scene;

      // UI pass.
      UILayerPtrArray layers;
      RenderJobArray uiRenderJobs;
      GetUIManager()->GetLayers(viewport->m_viewportId, layers);

      for (const UILayerPtr& layer : layers)
      {
        EntityRawPtrArray& uiNtties = layer->m_scene->AccessEntityArray();
        RenderJobProcessor::CreateRenderJobs(uiNtties, uiRenderJobs);
      }

      m_uiPass->m_params.OpaqueJobs.clear();
      m_uiPass->m_params.TranslucentJobs.clear();

      RenderJobProcessor::SeperateOpaqueTranslucent(uiRenderJobs,
                                                    m_uiPass->m_params.OpaqueJobs,
                                                    m_uiPass->m_params.TranslucentJobs);

      m_uiPass->m_params.Cam                                       = GetUIManager()->GetUICamera();
      m_uiPass->m_params.FrameBuffer                               = viewport->m_framebuffer;
      m_uiPass->m_params.ClearFrameBuffer                          = false;
      m_uiPass->m_params.ClearDepthBuffer                          = true;

      const EngineSettings::PostProcessingSettings& gfx            = GetEngineSettings().PostProcessing;

      // Bloom pass
      m_bloomPass->m_params.FrameBuffer                            = viewport->m_framebuffer;
      m_bloomPass->m_params.intensity                              = gfx.BloomIntensity;
      m_bloomPass->m_params.minThreshold                           = gfx.BloomThreshold;
      m_bloomPass->m_params.iterationCount                         = gfx.BloomIterationCount;

      // Light Complexity pass
      m_singleMatRenderer->m_params.ForwardParams.Cam              = m_camera;
      m_singleMatRenderer->m_params.ForwardParams.Lights           = lights;
      m_singleMatRenderer->m_params.ForwardParams.ClearFrameBuffer = true;

      m_singleMatRenderer->m_params.ForwardParams.OpaqueJobs       = renderJobs;

      m_singleMatRenderer->m_params.ForwardParams.FrameBuffer      = viewport->m_framebuffer;

      m_tonemapPass->m_params.FrameBuffer                          = viewport->m_framebuffer;
      m_tonemapPass->m_params.Method                               = gfx.TonemapperMode;

      // Gamma Pass.
      m_gammaPass->m_params.FrameBuffer                            = viewport->m_framebuffer;
      m_gammaPass->m_params.Gamma                                  = gfx.Gamma;

      // FXAA Pass
      m_fxaaPass->m_params.FrameBuffer                             = viewport->m_framebuffer;
      m_fxaaPass->m_params.screen_size                             = viewport->m_size;

      // Gizmo Pass.
      m_gizmoPass->m_params.Viewport                               = viewport;

      EditorBillboardBase* anchorGizmo                             = nullptr;
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
      m_lightSystem   = std::make_shared<ThreePointLightSystem>();

      // Create render mode materials.
      m_unlitOverride = GetMaterialManager()->GetCopyOfUnlitMaterial();
      m_unlitOverride->Init();

      m_billboardPass     = std::make_shared<BillboardPass>();
      m_scenePass         = std::make_shared<SceneRenderer>();
      m_uiPass            = std::make_shared<ForwardRenderPass>();
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
      auto RenderFn = [this, viewport, renderer](const EntityRawPtrArray& selection, const Vec4& color) -> void
      {
        if (selection.empty())
        {
          return;
        }

        RenderJobArray renderJobs;
        for (Entity* entity : selection)
        {
          // Disable light gizmos
          if (entity->IsLightInstance())
          {
            EnableLightGizmo(static_cast<Light*>(entity), false);
          }

          // Add billboards to draw list
          Entity* billboard = m_params.App->GetCurrentScene()->GetBillboard(entity);

          if (billboard)
          {
            static_cast<Billboard*>(billboard)->LookAt(viewport->GetCamera(), viewport->GetBillboardScale());

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

  } // namespace Editor
} // namespace ToolKit
