#pragma once

#include "Gizmo.h"
#include "Global.h"
#include "Pass.h"
#include "Primative.h"

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

    struct SingleMatSceneRenderPassParams
    {
      ForwardRenderPassParams ForwardParams;
      ShaderPtr OverrideFragmentShader;
    };

    // Render whole scene in forward renderer with a single override material
    struct SingleMatSceneRenderPass : public Pass
    {
     public:
      SingleMatSceneRenderPass();
      explicit SingleMatSceneRenderPass(const SingleMatSceneRenderPassParams& params);

      void Render() override;
      void PreRender() override;

     public:
      SingleMatSceneRenderPassParams m_params;

     private:
      /**
       * Override material for EditorLitMode::LightComplexity.
       */
      MaterialPtr m_overrideMat = nullptr;

      ForwardRenderPassPtr m_forwardPass    = nullptr;
    };

    typedef std::shared_ptr<SingleMatSceneRenderPass> LightComplexityPassPtr;

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
      class App* App                               = nullptr;
      class EditorViewport* Viewport               = nullptr;
      EditorLitMode LitMode                        = EditorLitMode::EditorLit;
      TonemapPassParams::TonemapMethod tonemapping = TonemapPassParams::Aces;
    };

    class Technique
    {
     public:
      virtual void Render() = 0;
    };

    class EditorRenderer : public Technique
    {
     public:
      EditorRenderer();
      explicit EditorRenderer(const EditorRenderPassParams& params);
      virtual ~EditorRenderer();

      void Render() override;
      void PreRender();
      void PostRender();
      void SetLitMode(EditorLitMode mode);
      static void CreateEditorLights(LightRawPtrArray& list, Node** parentNode);

     private:
      void InitRenderer();
      void OutlineSelecteds();

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
      LightRawPtrArray m_editorLights;
      /**
       * Parent Node that m_editorLights are attached to.
       */
      Node* m_lightNode                     = nullptr;
      /**
       * Override material for EditorLitMode::Unlit.
       */
      MaterialPtr m_unlitOverride           = nullptr;

      SceneRenderPass m_scenePass;
      TonemapPass m_tonemapPass;
      ForwardRenderPass m_editorPass;
      GizmoPass m_gizmoPass;
      GammaPass m_gammaPass;
      BloomPass m_bloomPass;
      OutlinePass m_outlinePass;
      SingleMatSceneRenderPass m_singleMatRenderer;
      Camera* m_camera = nullptr;

      /**
       * Selected entity list
       */
      EntityRawPtrArray m_selecteds;
    };

  } // namespace Editor
} // namespace ToolKit
