#pragma once

#include "Pass.h"

namespace ToolKit
{

  struct FullQuadPassParams
  {
    LightRawPtrArray lights    = {};
    FramebufferPtr FrameBuffer = nullptr;
    BlendFunction BlendFunc    = BlendFunction::NONE;
    ShaderPtr FragmentShader   = nullptr;
    bool ClearFrameBuffer      = true;
  };

  /**
   * Draws a full quad that covers entire FrameBuffer with given fragment
   * shader.
   */
  class TK_API FullQuadPass : public Pass
  {
   public:
    FullQuadPass();
    explicit FullQuadPass(const FullQuadPassParams& params);
    ~FullQuadPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    FullQuadPassParams m_params;
    MaterialPtr m_material;

   private:
    CameraPtr m_camera;
    QuadPtr m_quad;
  };

  typedef std::shared_ptr<FullQuadPass> FullQuadPassPtr;

} // namespace ToolKit