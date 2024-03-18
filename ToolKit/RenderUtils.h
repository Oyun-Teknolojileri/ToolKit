#pragma once

#include "Framebuffer.h"
#include "Renderer.h"
#include "TKOpenGL.h"

namespace ToolKit
{
  class TK_API RenderUtils
  {
   public:
    RenderUtils(Renderer* renderer);

    /**
     * @param absoluteFilePath This is the absolute folder path that the textures will be saved. Postfixes will be added
     * to this name such as _px, _py, _pz, _nx, _ny, _nz and mipmap level. Saves the given cubemap textures with hdr
     * format to the given path.
     *
     * NOTE: This function assumes that the gpu format of the given texture is RGBA with floating points.
     */
    bool SaveCubemapToFileRGBAFloat(const String& absoluteFilePath, CubeMapPtr cubemap, int mipmapLevel);

   private:
    RenderUtils() {}

    FramebufferPtr GetOneAttachmentFramebuffer();

   private:
    FramebufferPtr m_oneAttachmentFramebuffer = nullptr;
    Renderer* m_renderer                      = nullptr;
  };
} // namespace ToolKit