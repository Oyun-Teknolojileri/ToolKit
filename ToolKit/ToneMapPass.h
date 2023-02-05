#pragma once

#include "PostProcessPass.h"

namespace ToolKit
{

  enum class TonemapMethod
  {
    Reinhard,
    Aces
  };

  struct TonemapPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    TonemapMethod Method       = TonemapMethod::Aces;
  };

  class TK_API TonemapPass : public PostProcessPass
  {
   public:
    TonemapPass();
    explicit TonemapPass(const TonemapPassParams& params);
    ~TonemapPass();

    void PreRender() override;

   public:
    TonemapPassParams m_params;
  };

  typedef std::shared_ptr<TonemapPass> TonemapPassPtr;

} // namespace ToolKit