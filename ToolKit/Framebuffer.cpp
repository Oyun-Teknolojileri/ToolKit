#include "Framebuffer.h"

#include "GL/glew.h"
#include "ToolKit.h"

namespace ToolKit
{
  Framebuffer::Framebuffer()
  {
    for (int i = 0; i < m_maxColorAttachmentCount; ++i)
    {
      m_colorAtchs[i] = nullptr;
    }
  }

  Framebuffer::~Framebuffer()
  {
    UnInit();
  }

  void Framebuffer::Init(const FramebufferSettings& settings)
  {
    if (m_initialized)
    {
      return;
    }

    m_settings = settings;

    // If msaa is not supported, do not use
    if (glFramebufferTexture2DMultisampleEXT == nullptr)
    {
      m_settings.msaa = 0;
      GetLogger()->Log(
          "Unsupported Extension: glFramebufferTexture2DMultisampleEXT");
    }

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
      GLenum component  = GL_DEPTH_COMPONENT;
      if (m_settings.depthStencil)
      {
        component  = GL_DEPTH_STENCIL;
        attachment = GL_DEPTH_STENCIL_ATTACHMENT;
      }
#ifndef __EMSCRIPTEN__
      if (m_settings.msaa > 0)
      {
        glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER,
                                            m_settings.msaa,
                                            component,
                                            m_settings.width,
                                            m_settings.height);
      }
      else
#endif
      {
        glRenderbufferStorage(
            GL_RENDERBUFFER, component, m_settings.width, m_settings.height);
      }

      // Attach depth buffer to FBO
      glFramebufferRenderbuffer(
          GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, m_defaultRboId);

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

  RenderTarget* Framebuffer::SetAttachment(Attachment atc,
                                           RenderTarget* rt,
                                           CubemapFace face)
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
      return nullptr;
    }

    RenderTarget* oldRt = DetachAttachment(atc);

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
                             0);
    }
#ifndef __EMSCRIPTEN__
    else if (rt->m_settings.Msaa > 0 && m_settings.msaa == rt->m_settings.Msaa)
    {
      glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER,
                                           attachment,
                                           GL_TEXTURE_2D,
                                           rt->m_textureId,
                                           0,
                                           rt->m_settings.Msaa);
    }
#endif
    else
    {
      glFramebufferTexture(GL_FRAMEBUFFER, attachment, rt->m_textureId, 0);
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

  RenderTarget* Framebuffer::GetAttachment(Attachment atc)
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
    if (m_defaultRboId != 0)
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

  RenderTarget* Framebuffer::DetachAttachment(Attachment atc)
  {
    RenderTarget* rt  = m_depthAtch;
    GLenum attachment = GL_DEPTH_ATTACHMENT;
    if (IsColorAttachment(atc))
    {
      attachment = GL_COLOR_ATTACHMENT0 + (int) atc;

      rt = m_colorAtchs[(int) atc];
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
#ifndef __EMSCRIPTEN__
      if (rt->m_settings.Msaa > 0 && m_settings.msaa == rt->m_settings.Msaa)
      {
        glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER,
                                             attachment,
                                             GL_TEXTURE_2D,
                                             0,
                                             0,
                                             rt->m_settings.Msaa);
      }
      else
#endif
      {
        glFramebufferTexture(GL_FRAMEBUFFER, attachment, 0, 0);
      }

      if (IsColorAttachment(atc))
      {
        m_colorAtchs[(int) atc] = nullptr;
        SetDrawBuffers();
      }

      glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
    }

    return rt;
  }

  uint Framebuffer::GetFboId()
  {
    return m_fboId;
  }

  uint Framebuffer::GetDefaultRboId()
  {
    return m_defaultRboId;
  }

  FramebufferSettings Framebuffer::GetSettings()
  {
    return m_settings;
  }

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

    GLenum colorAttachments[8];
    int count = 0;
    for (int i = 0; i < m_maxColorAttachmentCount; ++i)
    {
      if (m_colorAtchs[i] != nullptr && m_colorAtchs[i]->m_textureId != 0)
      {
        colorAttachments[i] = GL_COLOR_ATTACHMENT0 + i;
        ++count;
      }
    }

    glDrawBuffers(count, colorAttachments);

    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
  }

  bool Framebuffer::IsColorAttachment(Attachment atc)
  {
    if (atc == Attachment::DepthAttachment ||
        atc == Attachment::DepthStencilAttachment)
    {
      return false;
    }
    else
    {
      return true;
    }
  }

} // namespace ToolKit
