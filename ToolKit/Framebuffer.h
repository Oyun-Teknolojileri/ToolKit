#pragma once

#include "Texture.h"

namespace ToolKit
{

  struct FramebufferSettings
  {
    uint width           = 1024;
    uint height          = 1024;
    int msaa             = 0;
    bool depthStencil    = false;
    bool useDefaultDepth = true;

    bool Compare(const FramebufferSettings& settings);
  };

  class TK_API Framebuffer
  {
    // NOTE: This class does not handle renderbuffer attachments, multi-sampled
    // cubemaps, texture arrays, stencil attachments(including depth_stencil).
    // NOTE: All mipmap levels set to 0. No support to set different levels of
    // mipmaps of textures.

    // Initalized framebuffer carries either depth attachment or depth
    // stencil attachment. In order to change, uninit and init the framebuffer
    // with new settings.

   public:
    enum class Attachment
    {
      ColorAttachment0       = 0,
      ColorAttachment1       = 1,
      ColorAttachment2       = 2,
      ColorAttachment3       = 3,
      ColorAttachment4       = 4,
      ColorAttachment5       = 5,
      ColorAttachment6       = 6,
      ColorAttachment7       = 7,
      DepthAttachment        = 100,
      DepthStencilAttachment = 101
    };

    enum class CubemapFace
    {
      POS_X = 0,
      NEG_X = 1,
      POS_Y = 2,
      NEG_Y = 3,
      POS_Z = 4,
      NEG_Z = 5,
      NONE
    };

   public:
    Framebuffer();
    ~Framebuffer();

    void Init(const FramebufferSettings& settings);
    void UnInit();
    bool Initialized();

    RenderTargetPtr SetAttachment(Attachment atc,
                                  RenderTargetPtr rt,
                                  int layer        = -1,
                                  CubemapFace face = CubemapFace::NONE);
    RenderTargetPtr GetAttachment(Attachment atc);
    void ClearAttachments();

    uint GetFboId();
    uint GetDefaultRboId();
    FramebufferSettings GetSettings();
    void CheckFramebufferComplete();

    void ReconstructIfNeeded(uint width, uint height);

   private:
    RenderTargetPtr DetachAttachment(Attachment atc);
    void DeleteDefaultDepthAttachment();
    void SetDrawBuffers();
    bool IsColorAttachment(Attachment atc);

   public:
    static const int m_maxColorAttachmentCount = 8;

   private:
    FramebufferSettings m_settings;

    bool m_initialized  = false;
    uint m_fboId        = 0;
    uint m_defaultRboId = 0;
    RenderTargetPtr m_colorAtchs[m_maxColorAttachmentCount];
    RenderTargetPtr m_depthAtch = nullptr;
  };

}; // namespace ToolKit
