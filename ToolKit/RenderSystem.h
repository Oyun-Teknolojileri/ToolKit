#pragma once

#include "Pass.h"
#include "Renderer.h"

namespace ToolKit
{

  /**
   * Base class responsible of creating render results using passes.
   */
  class TK_API Technique
  {
   public:
    Technique();
    virtual ~Technique();
    virtual void Render(Renderer* renderer);

   public:
    PassPtrArray m_passArray;
  };

  typedef std::shared_ptr<Technique> TechniquePtr;

  /**
   * System class that facilitates renderer to the techniques.
   */
  class TK_API RenderSystem
  {
   public:
    RenderSystem();
    ~RenderSystem();
    void Render(Technique* technique);
    void Render(TechniquePtr technique);

   private:
    Renderer* m_renderer         = nullptr;
    Technique* m_renderTechnique = nullptr;
  };

} // namespace ToolKit