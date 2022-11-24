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

     private:
      void InitRenderer();

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
      Node* m_lightNode = nullptr;
      /**
       * Override material for EditorLitMode::LightComplexity.
       */
      MaterialPtr m_lightComplexityOverride = nullptr;
      /**
       * Override material for EditorLitMode::LightingOnly.
       */
      MaterialPtr m_lightingOnlyOverride = nullptr;
      /**
       * Override material for EditorLitMode::Unlit.
       */
      MaterialPtr m_unlitOverride = nullptr;

      bool m_overrideDiffuseTexture = false;
      ShadowPass m_shadowPass;
      RenderPass m_scenePass;
      RenderPass m_editorPass;
      GizmoPass m_gizmoPass;
      Camera* m_camera             = nullptr;
      EditorScenePtr m_editorScene = nullptr;
    };

  } // namespace Editor
} // namespace ToolKit