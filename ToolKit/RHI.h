#pragma once

#include "TKOpenGL.h"
#include "TKStats.h"
#include "Types.h"

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

    // TODO hold the ids for draw & read framebuffer. If we try to set the same id twice for same slot, don't make api
    // call
  };
} // namespace ToolKit
