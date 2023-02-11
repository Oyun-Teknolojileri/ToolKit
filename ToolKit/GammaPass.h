#pragma once

#include "PostProcessPass.h"

namespace ToolKit
{

  struct GammaPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    float Gamma                = 2.2f;
  };

  /**
   * Apply gamma correction to given frame buffer.
   */
  class TK_API GammaPass : public PostProcessPass
  {
   public:
    GammaPass();
    explicit GammaPass(const GammaPassParams& params);
    ~GammaPass();

    void PreRender() override;

   public:
    GammaPassParams m_params;
  };

  typedef std::shared_ptr<GammaPass> GammaPassPtr;

} // namespace ToolKit