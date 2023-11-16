/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

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