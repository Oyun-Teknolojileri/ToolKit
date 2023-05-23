/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Framebuffer.h"

#include "ToolKit.h"
#include "gles2.h"

#include "DebugNew.h"

namespace ToolKit
{

  bool FramebufferSettings::Compare(const FramebufferSettings& settings)
  {
    return settings.width == width && settings.height == height && settings.depthStencil == depthStencil &&
           settings.useDefaultDepth == useDefaultDepth;
  }

  Framebuffer::Framebuffer()
  {
    for (int i = 0; i < m_maxColorAttachmentCount; ++i)
    {
      m_colorAtchs[i] = nullptr;
    }
  }

  Framebuffer::~Framebuffer() { UnInit(); }

  void Framebuffer::Init(const FramebufferSettings& settings)
  {
    if (m_initialized)
    {
      return;
    }

    m_settings = settings;

    // Create framebuffer object
    glGenFramebuffers(1, &m_fboId);

    if (settings.width == 0)
    {
      m_settings.width = 1024;
    }

    if (settings.height == 0)
    {
      m_settings.height = 1024;
    }

    if (m_settings.useDefaultDepth)
    {
      GLint lastFBO;
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);

      glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

      // Create a default depth, depth-stencil buffer
      glGenRenderbuffers(1, &m_defaultRboId);
      glBindRenderbuffer(GL_RENDERBUFFER, m_defaultRboId);

      GLenum attachment = GL_DEPTH_ATTACHMENT;
      GLenum component  = GL_DEPTH_COMPONENT24;
      if (m_settings.depthStencil)
      {
        component  = GL_DEPTH24_STENCIL8;
        attachment = GL_DEPTH_STENCIL_ATTACHMENT;
      }

      glRenderbufferStorage(GL_RENDERBUFFER, component, m_settings.width, m_settings.height);

      // Attach depth buffer to FBO
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, m_defaultRboId);

      // Check if framebuffer is complete
      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      {
        GetLogger()->Log("Error: Framebuffer incomplete!");
      }

      glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
    }

    m_initialized = true;
  }

  void Framebuffer::UnInit()
  {
    if (!m_initialized)
    {
      return;
    }

    ClearAttachments();

    // Delete framebuffer
    glDeleteFramebuffers(1, &m_fboId);

    m_initialized = false;
  }

  bool Framebuffer::Initialized() { return m_initialized; }

  void Framebuffer::ReconstructIfNeeded(uint width, uint height)
  {
    if (!m_initialized || m_settings.width != width || m_settings.height != height)
    {
      UnInit();
      m_settings.width  = width;
      m_settings.height = height;
      Init(m_settings);
    }
  }

  RenderTargetPtr Framebuffer::SetAttachment(Attachment atc, RenderTargetPtr rt, int mip, int layer, CubemapFace face)
  {
    GLenum attachment = GL_DEPTH_ATTACHMENT;
    if (IsColorAttachment(atc))
    {
      attachment = GL_COLOR_ATTACHMENT0 + (int) atc;
    }
    else
    {
      if (m_settings.depthStencil)
      {
        attachment = GL_DEPTH_STENCIL_ATTACHMENT;
      }
      DeleteDefaultDepthAttachment();
    }

    if (rt->m_width <= 0 || rt->m_height <= 0 || rt->m_textureId == 0)
    {
      assert(false && "Render target can't be bind.");
      return nullptr;
    }

    RenderTargetPtr oldRt = DetachAttachment(atc);

    GLint lastFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

    // Set attachment
    if (face != CubemapFace::NONE)
    {
      glFramebufferTexture2D(GL_FRAMEBUFFER,
                             attachment,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + (int) face,
                             rt->m_textureId,
                             mip);
    }
    else
    {
      if (layer != -1)
      {
        assert(layer < rt->m_settings.Layers);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, rt->m_textureId, mip, layer);
      }
      else
      {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, rt->m_textureId, mip);
      }
    }

    if (!IsColorAttachment(atc))
    {
      m_depthAtch = rt;
    }
    else
    {
      m_colorAtchs[(int) atc] = rt;
      SetDrawBuffers();
    }

    CheckFramebufferComplete();

    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);

    m_settings.width  = rt->m_width;
    m_settings.height = rt->m_height;

    return oldRt;
  }

  RenderTargetPtr Framebuffer::GetAttachment(Attachment atc)
  {
    if (IsColorAttachment(atc))
    {
      return m_colorAtchs[(int) atc];
    }
    else
    {
      return m_depthAtch;
    }
  }

  void Framebuffer::ClearAttachments()
  {
    // Detach all attachments
    if (m_defaultRboId == 0)
    {
      DetachAttachment(Attachment::DepthAttachment);
    }
    else
    {
      DeleteDefaultDepthAttachment();
    }
    for (int i = 0; i < m_maxColorAttachmentCount; ++i)
    {
      if (m_colorAtchs[i] != nullptr)
      {
        DetachAttachment((Attachment) i);
        m_colorAtchs[i] = nullptr;
      }
    }
  }

  RenderTargetPtr Framebuffer::DetachAttachment(Attachment atc)
  {
    RenderTargetPtr rt = m_depthAtch;
    GLenum attachment  = GL_DEPTH_ATTACHMENT;
    if (IsColorAttachment(atc))
    {
      attachment = GL_COLOR_ATTACHMENT0 + (int) atc;

      rt         = m_colorAtchs[(int) atc];
    }
    else if (atc == Attachment::DepthStencilAttachment)
    {
      attachment = GL_DEPTH_STENCIL_ATTACHMENT;
    }

    if (rt != nullptr)
    {
      GLint lastFBO;
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);

      // Detach
      glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
      glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, 0, 0);

      if (IsColorAttachment(atc))
      {
        m_colorAtchs[(int) atc] = nullptr;
        SetDrawBuffers();
      }

      glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
    }

    return rt;
  }

  uint Framebuffer::GetFboId() { return m_fboId; }

  uint Framebuffer::GetDefaultRboId() { return m_defaultRboId; }

  FramebufferSettings Framebuffer::GetSettings() { return m_settings; }

  void Framebuffer::CheckFramebufferComplete()
  {
    GLint lastFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
    GLenum check = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(check == GL_FRAMEBUFFER_COMPLETE && "Framebuffer incomplete");
    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
  }

  void Framebuffer::DeleteDefaultDepthAttachment()
  {
    if (m_defaultRboId == 0)
    {
      return;
    }

    GLint lastFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

    GLenum attachment = GL_DEPTH_ATTACHMENT;
    if (m_settings.depthStencil)
    {
      attachment = GL_DEPTH_STENCIL_ATTACHMENT;
    }

    // Detach depth buffer from FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);

    glDeleteRenderbuffers(1, &m_defaultRboId);
    m_defaultRboId = 0;
  }

  void Framebuffer::SetDrawBuffers()
  {
    GLint lastFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

    GLenum colorAttachments[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int count                  = 0;
    for (int i = 0; i < m_maxColorAttachmentCount; ++i)
    {
      if (m_colorAtchs[i] != nullptr && m_colorAtchs[i]->m_textureId != 0)
      {
        colorAttachments[i] = GL_COLOR_ATTACHMENT0 + i;
        ++count;
      }
    }

    if (count == 0)
    {
      glDrawBuffers(0, nullptr);
    }
    else
    {
      glDrawBuffers(count, colorAttachments);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
  }

  bool Framebuffer::IsColorAttachment(Attachment atc)
  {
    if (atc == Attachment::DepthAttachment || atc == Attachment::DepthStencilAttachment)
    {
      return false;
    }
    else
    {
      return true;
    }
  }

} // namespace ToolKit
