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

#ifdef __EMSCRIPTEN__
  #include <GL/glew.h>
#else
  #include <gles2.h>
#endif

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
    m_depthAtch = nullptr;
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
      m_depthAtch = MakeNewPtr<DepthTexture>();
      m_depthAtch->Init(m_settings.width, m_settings.height, m_settings.depthStencil);
      AttachDepthTexture(m_depthAtch);
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

    glDeleteFramebuffers(1, &m_fboId);
    m_fboId       = 0;
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

  void Framebuffer::AttachDepthTexture(DepthTexturePtr dt)
  {
    m_depthAtch = dt;

    GLint lastFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

    GLenum attachment = dt->m_stencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;

    // Attach depth buffer to FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, dt->m_textureId);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      GetLogger()->Log("Error: Framebuffer incomplete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
  }

  DepthTexturePtr Framebuffer::GetDepthTexture() { return m_depthAtch; }

  RenderTargetPtr Framebuffer::SetAttachment(Attachment atc, RenderTargetPtr rt, int mip, int layer, CubemapFace face)
  {
    GLenum attachment = GL_DEPTH_ATTACHMENT;
    attachment        = GL_COLOR_ATTACHMENT0 + (int) atc;

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

    m_colorAtchs[(int) atc] = rt;
    SetDrawBuffers();

    CheckFramebufferComplete();

    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);

    m_settings.width  = rt->m_width;
    m_settings.height = rt->m_height;

    return oldRt;
  }

  RenderTargetPtr Framebuffer::GetAttachment(Attachment atc) { return m_colorAtchs[(int) atc]; }

  void Framebuffer::ClearAttachments()
  {
    if (!m_initialized)
    {
      return;
    }
    // Detach all attachments
    RemoveDepthAttachment();

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
    RenderTargetPtr rt = m_colorAtchs[(int) atc];
    if (rt == nullptr)
    {
      return nullptr;
    }

    GLint lastFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

    GLenum attachment = GL_COLOR_ATTACHMENT0 + (int) atc;
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, 0, 0); // Detach

    m_colorAtchs[(int) atc] = nullptr;
    SetDrawBuffers();

    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
    return rt;
  }

  uint Framebuffer::GetFboId() { return m_fboId; }

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

  void Framebuffer::RemoveDepthAttachment()
  {
    if (m_depthAtch == nullptr)
    {
      return;
    }
    GLint lastFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

    GLenum attachment = m_settings.depthStencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;

    // Detach depth buffer from FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, 0);

    if (m_settings.useDefaultDepth)
    {
      m_depthAtch->UnInit();
    }
    m_depthAtch = nullptr;
    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
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
