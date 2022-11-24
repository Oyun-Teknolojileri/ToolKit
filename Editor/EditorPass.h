#pragma once

#include "Gizmo.h"
#include "Pass.h"
#include "Primative.h"

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
      LightingOnly
    };

    struct EditorRenderPassParams
    {
      class App* App                 = nullptr;
      class EditorViewport* Viewport = nullptr;
      EditorLitMode LitMode          = EditorLitMode::EditorLit;
    };

    class EditorRenderPass : public RenderPass
    {
     public:
      EditorRenderPass();
      explicit EditorRenderPass(const EditorRenderPassParams& params);
      virtual ~EditorRenderPass();

      void Render() override;
      void PreRender() override;
      void PostRender() override;
      void SetLitMode(EditorLitMode mode);

     private:
      void InitPass();

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
    };

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

  } // namespace Editor
} // namespace ToolKit
