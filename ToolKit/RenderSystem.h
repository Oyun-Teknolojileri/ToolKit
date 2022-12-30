#pragma once

#include "Pass.h"
#include "PostProcessPass.h"
#include "Renderer.h"

namespace ToolKit
{

  class TK_API Technique
  {
   public:
    Technique();
    virtual ~Technique();
    virtual void Render(Renderer* renderer);

   public:
    PassPtrArray m_passArray;
  };

  class TK_API RenderSystem
  {
   public:
    RenderSystem();
    ~RenderSystem();
    void Render(Technique* technique);

   private:
    Renderer* m_renderer         = nullptr;
    Technique* m_renderTechnique = nullptr;
  };

} // namespace ToolKit