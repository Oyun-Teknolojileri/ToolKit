#include "RenderUtils.h"

#include "FileManager.h"
#include "TKImage.h"
#include "ToolKit.h"
#include "Types.h"

namespace ToolKit
{
  RenderUtils::RenderUtils(Renderer* renderer)
  {
    m_renderer = renderer;
    assert(m_renderer != nullptr);
  }

  bool RenderUtils::SaveCubemapToFileRGBAFloat(const String& absoluteFilePath, CubeMapPtr cubemap, int mipmapLevel)
  {
    if (cubemap->m_textureId == 0 || m_renderer == nullptr)
    {
      return false;
    }

    // temporarily set this texture as render target
    RenderTargetPtr tempRenderTarget = MakeNewPtr<RenderTarget>();
    tempRenderTarget->Settings(cubemap->Settings());
    tempRenderTarget->m_textureId = cubemap->m_textureId;
    tempRenderTarget->m_initiated = true;
    tempRenderTarget->m_width     = cubemap->m_width;
    tempRenderTarget->m_height    = cubemap->m_height;

    auto exitFn                   = [&tempRenderTarget]()
    {
      // avoid destructing the gpu texture
      tempRenderTarget->m_initiated = false;
      tempRenderTarget->m_textureId = 0;
    };

    // if directory is not there, create one
    GetFileManager()->CreateDirectory(ConcatPaths({ResourcePath(), "IBLTexturesCache"}));

    int width         = cubemap->m_width / ((int) std::pow(2, mipmapLevel));
    int height        = cubemap->m_height / ((int) std::pow(2, mipmapLevel));

    // Prepare framebuffer && cpu buffer
    FramebufferPtr fb = GetOneAttachmentFramebuffer();
    m_renderer->SetFramebuffer(fb, GraphicBitFields::None);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    GLfloat* pixelData = new GLfloat[width * height * BytesOfFormat(tempRenderTarget->Settings().InternalFormat)];
    const StringArray postfixes = {"_px", "_nx", "_py", "_ny", "_pz", "_nz"};

    {
      for (int i = 0; i < 6; ++i)
      {
        fb->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0,
                               tempRenderTarget,
                               mipmapLevel,
                               -1,
                               (Framebuffer::CubemapFace) i);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, pixelData);

        String filePath = absoluteFilePath + postfixes[i] + "_" + std::to_string(mipmapLevel) + ".hdr";
        WriteHDR(filePath.c_str(), width, height, 4, pixelData);
      }
    }

    SafeDelArray(pixelData);

    exitFn();

    return true;
  }

  FramebufferPtr RenderUtils::GetOneAttachmentFramebuffer()
  {
    if (m_oneAttachmentFramebuffer == nullptr)
    {
      m_oneAttachmentFramebuffer = MakeNewPtr<Framebuffer>();
      m_oneAttachmentFramebuffer->Init({0, 0, false, false});
    }

    return m_oneAttachmentFramebuffer;
  }
} // namespace ToolKit