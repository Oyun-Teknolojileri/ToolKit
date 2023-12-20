#pragma once

#include "TKOpenGL.h"
#include "Types.h"
#include "TKStats.h"

namespace ToolKit
{
  class RHI
  {
    friend class Renderer;
    friend class Framebuffer;
    friend class RenderSystem;

   private:
    TK_API static inline void SetFramebuffer(GLenum target, GLuint framebufferID)
    {
      glBindFramebuffer(target, framebufferID);
      m_currentFramebufferID = framebufferID;

      if (target != GL_READ_FRAMEBUFFER)
      {
        AddHWRenderPass();
      }
    }

   private:
    static GLuint m_currentFramebufferID;
  };
} // namespace ToolKit
