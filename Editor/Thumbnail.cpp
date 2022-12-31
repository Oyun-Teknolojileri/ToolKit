#include "Thumbnail.h"

#include "stdafx.h"

namespace ToolKit
{

  namespace Editor
  {
    ThumbnailRenderer::ThumbnailRenderer()
    {
      FramebufferSettings fbs;
      fbs.width         = m_maxThumbSize;
      fbs.height        = m_maxThumbSize;
      m_thumbnailBuffer = std::make_shared<Framebuffer>(fbs);

      m_thumbnailRT =
          std::make_shared<RenderTarget>(m_maxThumbSize, m_maxThumbSize);

      m_thumbnailBuffer->SetAttachment(
          Framebuffer::Attachment::ColorAttachment0,
          m_thumbnailRT);

      m_sphere = std::make_shared<Sphere>();
      m_cam    = std::make_shared<Camera>();
    }

    ThumbnailRenderer::~ThumbnailRenderer()
    {
      m_thumbnailBuffer = nullptr;
      m_thumbnailRT     = nullptr;
      m_thumbnailScene  = nullptr;
      m_sphere          = nullptr;
      m_cam             = nullptr;
    }

    RenderTargetPtr ThumbnailRenderer::RenderThumbnail(const String& fullPath)
    {
      return RenderTargetPtr();
    }

    void ThumbnailRenderer::PreRender() {}

  } // namespace Editor

} // namespace ToolKit
