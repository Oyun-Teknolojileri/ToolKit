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

  GLuint RHI::m_currentVAO               = -1; // max unsigned integer

  std::unordered_map<uint, uint> RHI::m_slotTextureIDmap;

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

  void RHI::SetTexture(GLenum target, GLuint textureID, GLenum textureSlot)
  {
    assert(textureSlot >= 0 && textureSlot <= 31);

    auto itr = m_slotTextureIDmap.find(textureSlot);
    if (itr != m_slotTextureIDmap.end())
    {
      if (itr->second == textureID)
      {
        // Do not bind if already binded.
        return;
      }
    }

    m_slotTextureIDmap[textureSlot] = textureID;
    glActiveTexture(GL_TEXTURE0 + textureSlot);
    glBindTexture(target, textureID);
  }

  void RHI::DeleteTextures(int textureCount, GLuint* textures)
  {
    for (int i = 0; i < textureCount; ++i)
    {
      if (m_slotTextureIDmap.find(textures[i]) != m_slotTextureIDmap.end())
      {
        m_slotTextureIDmap[textures[i]] = 0;
      }
    }

    glDeleteTextures(textureCount, textures);
  }

  void RHI::BindVertexArray(GLuint VAO)
  {
    if (m_currentVAO != VAO)
    {
      glBindVertexArray(VAO);

      m_currentVAO = VAO;
    }
  }

} // namespace ToolKit
