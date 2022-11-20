#pragma once

#include "Pass.h"
#include "Primative.h"
#include "Gizmo.h"

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
