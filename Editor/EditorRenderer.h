#pragma once

#include "BillboardPass.h"
#include "EditorLight.h"
#include "Gizmo.h"
#include "GizmoPass.h"
#include "Global.h"
#include "OutlinePass.h"
#include "Pass.h"
#include "PostProcessPass.h"
#include "Primative.h"
#include "RenderSystem.h"
#include "SceneRenderer.h"
#include "SingleMaterialPass.h"

namespace ToolKit
{
  namespace Editor
  {

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
      LightingOnly,
      /**
       * Renders scene exactly as in game.
       */
      Game
    };

    struct EditorRenderParams
    {
      class App* App                 = nullptr;
      class EditorViewport* Viewport = nullptr;
      EditorLitMode LitMode          = EditorLitMode::EditorLit;
    };

    class EditorRenderer : public Technique
    {
     public:
      EditorRenderer();
      explicit EditorRenderer(const EditorRenderParams& params);
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
      EditorRenderParams m_params;

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

      BillboardPassPtr m_billboardPass                  = nullptr;
      SceneRendererPtr m_scenePass                      = nullptr;
      ForwardRenderPassPtr m_editorPass                 = nullptr;
      GizmoPassPtr m_gizmoPass                          = nullptr;
      TonemapPassPtr m_tonemapPass                      = nullptr;
      GammaPassPtr m_gammaPass                          = nullptr;
      BloomPassPtr m_bloomPass                          = nullptr;
      SSAOPassPtr m_ssaoPass                            = nullptr;
      OutlinePassPtr m_outlinePass                      = nullptr;
      FXAAPassPtr m_fxaaPass                            = nullptr;
      SingleMatForwardRenderPassPtr m_singleMatRenderer = nullptr;
      Camera* m_camera                                  = nullptr;

      /**
       * Selected entity list
       */
      EntityRawPtrArray m_selecteds;
    };

  } // namespace Editor
} // namespace ToolKit
