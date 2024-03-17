#pragma once

#include "Viewport.h"

namespace ToolKit
{
  class TK_API GameViewport : public Viewport
  {
   public:
    GameViewport();
    GameViewport(float width, float height);

    void Update(float dt) override;
  };
} // namespace ToolKit
