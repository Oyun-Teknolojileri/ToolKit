#pragma once
#include "Pass.h"
#include "StencilPass.h"

namespace ToolKit
{

  struct OutlinePassParams
  {
    EntityRawPtrArray DrawList = {};
    FramebufferPtr FrameBuffer = nullptr;
    Camera* Camera             = nullptr;
    Vec4 OutlineColor          = Vec4(1.0f);
  };

  /**
   * Draws given entities' outlines to the FrameBuffer.
   * TODO: It should be Technique instead of Pass
   */
  class TK_API OutlinePass : public Pass
  {
   public:
    OutlinePass();
    explicit OutlinePass(const OutlinePassParams& params);
    ~OutlinePass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    OutlinePassParams m_params;

   private:
    StencilRenderPassPtr m_stencilPass = nullptr;
    FullQuadPassPtr m_outlinePass      = nullptr;
    ShaderPtr m_dilateShader           = nullptr;
    RenderTargetPtr m_stencilAsRt      = nullptr;
  };

  typedef std::shared_ptr<OutlinePass> OutlinePassPtr;

} // namespace ToolKit