#pragma once

#include "EditorLight.h"
#include "Gizmo.h"
#include "Global.h"
#include "Pass.h"
#include "PostProcessPass.h"
#include "Primative.h"
#include "RenderSystem.h"
#include "SceneRenderer.h"

namespace ToolKit
{
  namespace Editor
  {

    struct GizmoPassParams
    {
      Viewport* Viewport = nullptr;
      std::vector<EditorBillboardBase*> GizmoArray;
    };

    class GizmoPass : public Pass
    {
     public:
      GizmoPass();
      explicit GizmoPass(const GizmoPassParams& params);

      void Render() override;
      void PreRender() override;
      void PostRender() override;

     public:
      GizmoPassParams m_params;

     private:
      SpherePtr m_depthMaskSphere = nullptr;
      Camera* m_camera            = nullptr;
    };

    typedef std::shared_ptr<GizmoPass> GizmoPassPtr;

    struct SingleMatForwardRenderPassParams
    {
      ForwardRenderPassParams ForwardParams;
      ShaderPtr OverrideFragmentShader;
    };

    // Render whole scene in forward renderer with a single override material
    struct SingleMatForwardRenderPass : public ForwardRenderPass
    {
     public:
      SingleMatForwardRenderPass();
      explicit SingleMatForwardRenderPass(
          const SingleMatForwardRenderPassParams& params);

      void Render() override;
      void PreRender() override;

     public:
      SingleMatForwardRenderPassParams m_params;

     private:
      MaterialPtr m_overrideMat = nullptr;
    };

    typedef std::shared_ptr<SingleMatForwardRenderPass>
        SingleMatForwardRenderPassPtr;

    /**
     * Enumeration for available render modes for the editor.
     */
    enum class EditorLitMode
    {
      /**
       * Uses editor lights which is a three point lighting system attached to
       * camera. Always lit where the viewport camera looks at.
       */
      EditorLit,
      /**
       * Renders scene with albedo colors only.
       */
      Unlit,
      /**
       * Uses lights in the scene with all rendering features enabled.
       */
      FullyLit,
      /**
       * Shows a heat map based on number of lights illuminating the object.
       */
      LightComplexity,
      /**
       * Shows lighting result with a white material assigned to all objects.
       */
      LightingOnly
    };

    struct EditorRenderPassParams
    {
      class App* App                 = nullptr;
      class EditorViewport* Viewport = nullptr;
      EditorLitMode LitMode          = EditorLitMode::EditorLit;
      TonemapMethod tonemapping      = TonemapMethod::Aces;
    };

    class EditorRenderer : public Technique
    {
     public:
      EditorRenderer();
      explicit EditorRenderer(const EditorRenderPassParams& params);
      virtual ~EditorRenderer();

      void Render(Renderer* renderer) override;
      void PreRender();
      void PostRender();

     private:
      void SetLitMode(Renderer* renderer, EditorLitMode mode);
      void InitRenderer();
      void OutlineSelecteds(Renderer* renderer);

     public:
      /**
       * Pass parameters.
       */
      EditorRenderPassParams m_params;

     private:
      /**
       * Three point lighting system which is used to illuminate the scene in
       * EditorLit mode.
       */
      ThreePointLightSystemPtr m_lightSystem            = nullptr;

      /**
       * Override material for EditorLitMode::Unlit.
       */
      MaterialPtr m_unlitOverride                       = nullptr;

      SceneRendererPtr m_scenePass                      = nullptr;
      ForwardRenderPassPtr m_editorPass                 = nullptr;
      GizmoPassPtr m_gizmoPass                          = nullptr;
      TonemapPassPtr m_tonemapPass                      = nullptr;
      GammaPassPtr m_gammaPass                          = nullptr;
      BloomPassPtr m_bloomPass                          = nullptr;
      SSAOPassPtr m_ssaoPass                            = nullptr;
      OutlinePassPtr m_outlinePass                      = nullptr;
      SingleMatForwardRenderPassPtr m_singleMatRenderer = nullptr;
      Camera* m_camera                                  = nullptr;

      /**
       * Selected entity list
       */
      EntityRawPtrArray m_selecteds;
    };

  } // namespace Editor
} // namespace ToolKit
