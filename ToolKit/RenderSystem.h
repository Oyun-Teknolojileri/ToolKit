#pragma once

#include "Pass.h"
#include "PostProcessPass.h"
#include "Renderer.h"

namespace ToolKit
{

  class Technique
  {
   public:
    virtual void Render(Renderer* renderer) = 0;
  };

  class RenderSystem
  {
   public:

   public:
    Renderer* m_renderer         = nullptr;
    Technique* m_renderTechnique = nullptr;
  };

} // namespace ToolKit