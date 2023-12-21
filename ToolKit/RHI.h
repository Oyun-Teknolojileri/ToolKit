#pragma once

#include "TKOpenGL.h"
#include "TKStats.h"
#include "Types.h"

namespace ToolKit
{
  class TK_API RHI
  {
    friend class Renderer;
    friend class Framebuffer;
    friend class RenderSystem;

  private:
      static GLuint m_currentReadFramebufferID;
      static GLuint m_currentDrawFramebufferID;
      static GLuint m_currentFramebufferID;

   private:
    static inline void SetFramebuffer(GLenum target, GLuint framebufferID)
    {
      if (target == GL_READ_FRAMEBUFFER)
      {
        if (m_currentReadFramebufferID == framebufferID)
        {
          return;
        }
        else
        {
          m_currentReadFramebufferID = framebufferID;
        }
      }
      else if (target == GL_DRAW_FRAMEBUFFER)
      {
        if (m_currentDrawFramebufferID == framebufferID)
        {
          return;
        }
        else
        {
          m_currentDrawFramebufferID = framebufferID;
        }
      }
      else
      {
        if (m_currentFramebufferID == framebufferID && m_currentReadFramebufferID == framebufferID &&
            m_currentDrawFramebufferID == framebufferID)
        {
          return;
        }
        else
        {
          m_currentFramebufferID     = framebufferID;
          m_currentReadFramebufferID = framebufferID;
          m_currentDrawFramebufferID = framebufferID;
        }
      }

      glBindFramebuffer(target, framebufferID);
      AddHWRenderPass();
    }

    static inline void DeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
    {
      glDeleteFramebuffers(n, framebuffers);

      for (int i = 0; i < n; ++i)
      {
        if (framebuffers[i] == m_currentFramebufferID)
        {
          m_currentFramebufferID = -1;
        }

        if (framebuffers[i] == m_currentReadFramebufferID)
        {
          m_currentReadFramebufferID = -1;
        }

        if (framebuffers[i] == m_currentDrawFramebufferID)
        {
          m_currentDrawFramebufferID = -1;
        }
      }
    }

    static inline void InvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments)
    {
      glInvalidateFramebuffer(target, numAttachments, attachments);
    }
  };
} // namespace ToolKit
