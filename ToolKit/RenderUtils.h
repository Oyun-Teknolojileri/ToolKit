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
     * @param absoluteFilePath This is the absolute file path that the textures will be saved. Postfixes will be added
     * to this name such as _px, _py, _pz, _nx, _ny, _nz and mipmap level. Saves the given cubemap textures with hdr
     * format to the given path.
     *
     * The filename will creates with this format: absoluteFilePath + _<direction postfix>__<miplevel>.hdr
     * Example:
     * absoluteFilePath = "test/file/path/filename"
     * mipmalLevel = 2
     * Created File Path:  "test/file/path/filename_px_2.hdr"
     *
     * NOTE: This function assumes that the gpu format of the given texture is RGBA with 16 bit floating points.
     */
    bool SaveCubemapToFileRGBAFloat(const String& absoluteFilePath, CubeMapPtr cubemap, int mipmapLevel);

    /**
     * @param absoluteFilePath This is the absolute file path that the cubemap will be read. You do not need to give
     * direction postfixes (_px, _py, etc.) or extension. The filename is being generated automatically from arguments.
     * Read SaveCubemapToFileRGBAFloat() comment to see file name format.
     *
     * NOTE: This function assumes that the gpu format of the given texture is RGBA with 16 bit floating points.
     */
    bool ReadCubemapFromFileRGBAFloat(const String& absoluteFilePath, CubeMapPtr cubemap, int mipmapLevel);

   private:
    RenderUtils() {}

    FramebufferPtr GetOneAttachmentFramebuffer();

   private:
    FramebufferPtr m_oneAttachmentFramebuffer = nullptr;
    Renderer* m_renderer                      = nullptr;
  };
} // namespace ToolKit