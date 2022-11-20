#pragma once

#include "Pass.h"

namespace ToolKit
{
  namespace Editor
  {

    struct EditorRenderPassParams
    {
      class App* App                 = nullptr;
      class EditorViewport* Viewport = nullptr;
      FramebufferPtr FrameBuffer     = nullptr;
    };

    class EditorRenderPass : public RenderPass
    {
      void Render() override;
      void PreRender() override;
      void PostRender() override;

     public:
      EditorRenderPassParams m_params;
    };

  } // namespace Editor
} // namespace ToolKit
