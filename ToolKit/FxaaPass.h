#pragma once

#include "PostProcessPass.h"

namespace ToolKit
{

  struct FXAAPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    Vec2 screen_size;
  };

  class TK_API FXAAPass : public PostProcessPass
  {
   public:
    FXAAPass();
    explicit FXAAPass(const FXAAPassParams& params);

    void PreRender() override;

   public:
    FXAAPassParams m_params;
  };

  typedef std::shared_ptr<FXAAPass> FXAAPassPtr;

} // namespace ToolKit