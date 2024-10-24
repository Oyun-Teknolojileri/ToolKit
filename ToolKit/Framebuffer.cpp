/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Framebuffer.h"

#include "Logger.h"
#include "RHI.h"
#include "TKOpenGL.h"
#include "TKProfiler.h"
#include "TKStats.h"
#include "ToolKit.h"



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

    RHI::DeleteFramebuffers(1, &m_fboId);
    m_fboId       = 0;
    m_initialized = false;
  }

  bool Framebuffer::Initialized() { return m_initialized; }

  void Framebuffer::ReconstructIfNeeded(int width, int height)
  {
    CPU_FUNC_RANGE();

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
    CPU_FUNC_RANGE();

    m_depthAtch = dt;

    RHI::SetFramebuffer(GL_FRAMEBUFFER, m_fboId);

    GLenum attachment = dt->m_stencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;

    // Attach depth buffer to FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, dt->m_textureId);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      GetLogger()->Log("Error: Framebuffer incomplete!");
    }
  }

  DepthTexturePtr Framebuffer::GetDepthTexture() { return m_depthAtch; }

  RenderTargetPtr Framebuffer::SetColorAttachment(Attachment atc,
                                                  RenderTargetPtr rt,
                                                  int mip,
                                                  int layer,
                                                  CubemapFace face)
  {
    CPU_FUNC_RANGE();

    GLenum attachment = GL_COLOR_ATTACHMENT0 + (int) atc;

    if (rt->m_width <= 0 || rt->m_height <= 0 || rt->m_textureId == 0)
    {
      assert(false && "Render target can't be bind.");
      return nullptr;
    }

    RenderTargetPtr oldRt = m_colorAtchs[(int) atc];

    RHI::SetFramebuffer(GL_FRAMEBUFFER, m_fboId);

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
        assert(layer < rt->Settings().Layers);
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

    m_settings.width  = rt->m_width;
    m_settings.height = rt->m_height;

    return oldRt;
  }

  RenderTargetPtr Framebuffer::GetAttachment(Attachment atc)
  {
    assert(atc < Attachment::DepthAttachment);
    return m_colorAtchs[(int) atc];
  }

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
        DetachColorAttachment((Attachment) i);
        m_colorAtchs[i] = nullptr;
      }
    }
  }

  RenderTargetPtr Framebuffer::DetachColorAttachment(Attachment atc)
  {
    CPU_FUNC_RANGE();

    RenderTargetPtr rt = m_colorAtchs[(int) atc];
    if (rt == nullptr)
    {
      return nullptr;
    }

    RHI::SetFramebuffer(GL_FRAMEBUFFER, m_fboId);

    GLenum attachment = GL_COLOR_ATTACHMENT0 + (int) atc;
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, 0, 0); // Detach

    m_colorAtchs[(int) atc] = nullptr;
    SetDrawBuffers();

    return rt;
  }

  uint Framebuffer::GetFboId() { return m_fboId; }

  void Framebuffer::CheckFramebufferComplete()
  {
    CPU_FUNC_RANGE();

    RHI::SetFramebuffer(GL_FRAMEBUFFER, m_fboId);

    GLenum check = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(check == GL_FRAMEBUFFER_COMPLETE && "Framebuffer incomplete");
  }

  void Framebuffer::RemoveDepthAttachment()
  {
    CPU_FUNC_RANGE();

    if (m_depthAtch == nullptr)
    {
      return;
    }

    RHI::SetFramebuffer(GL_FRAMEBUFFER, m_fboId);

    GLenum attachment = m_settings.depthStencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;

    // Detach depth buffer from FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, 0);

    if (m_settings.useDefaultDepth)
    {
      m_depthAtch->UnInit();
    }
    m_depthAtch = nullptr;
  }

  void Framebuffer::SetDrawBuffers()
  {
    RHI::SetFramebuffer(GL_FRAMEBUFFER, m_fboId);

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
