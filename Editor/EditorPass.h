#pragma once

#include "Gizmo.h"
#include "Pass.h"
#include "Primative.h"

namespace ToolKit
{
  namespace Editor
  {

    struct EditorRenderPassParams
    {
      class App* App                 = nullptr;
      class EditorViewport* Viewport = nullptr;
    };

    class EditorRenderPass : public RenderPass
    {
     public:
      void Render() override;
      void PreRender() override;
      void PostRender() override;

     public:
      EditorRenderPassParams m_params;
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

    struct StencilRenderPassParams
    {
      RenderTargetPtr OutputTarget;
    };

    /**
     * Creates a binary stencil buffer from the given entities and copies the
     * binary image to OutputTarget.
     */
    class StencilRenderPass : public Pass
    {
     public:
      StencilRenderPass();
      explicit StencilRenderPass(const StencilRenderPassParams& params);

      void Render() override;
      void PreRender() override;
      void PostRender() override;

     public:
      StencilRenderPassParams m_params;

     private:
      FramebufferPtr m_frameBuffer = nullptr;
    };

  } // namespace Editor
} // namespace ToolKit
