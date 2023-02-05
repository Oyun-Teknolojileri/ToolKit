#pragma once

#include "Pass.h"
#include "FullQuadPass.h"

namespace ToolKit
{

  struct StencilRenderPassParams
  {
    RenderTargetPtr OutputTarget;
    EntityRawPtrArray DrawList;
    Camera* Camera = nullptr;
  };

  /**
   * Creates a binary stencil buffer from the given entities and copies the
   * binary image to OutputTarget.
   */
  class TK_API StencilRenderPass : public Pass
  {
   public:
    StencilRenderPass();
    explicit StencilRenderPass(const StencilRenderPassParams& params);
    ~StencilRenderPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    StencilRenderPassParams m_params;

   private:
    FramebufferPtr m_frameBuffer         = nullptr;
    MaterialPtr m_solidOverrideMaterial  = nullptr;
    FullQuadPassPtr m_copyStencilSubPass = nullptr;
  };

  typedef std::shared_ptr<StencilRenderPass> StencilRenderPassPtr;

} // namespace ToolKit