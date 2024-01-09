/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "RHI.h"

namespace ToolKit
{

  GLuint RHI::m_currentReadFramebufferID = -1; // max unsigned integer
  GLuint RHI::m_currentDrawFramebufferID = -1; // max unsigned integer
  GLuint RHI::m_currentFramebufferID     = -1; // max unsigned integer

  void RHI::SetFramebuffer(GLenum target, GLuint framebufferID)
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

  void RHI::DeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
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

  void RHI::InvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments)
  {
    glInvalidateFramebuffer(target, numAttachments, attachments);
  }

} // namespace ToolKit
