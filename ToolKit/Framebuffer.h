/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Texture.h"

namespace ToolKit
{

  struct FramebufferSettings
  {
    /**Height of the frame buffer. */
    int width                  = 128;
    /** Width of the frame buffer. */
    int height                 = 128;
    /** States whether the default depth has stencil or not. */
    bool depthStencil          = false;
    /** Creates a default depth attachment. */
    bool useDefaultDepth       = true;
    /** Creates multi sample frame buffers. Suggested values are 0, 2, 4, 6. */
    int multiSampleFrameBuffer = 0;

    bool operator==(const FramebufferSettings& other) const
    {
      return memcmp(this, &other, sizeof(FramebufferSettings)) == 0;
    }

    bool operator!=(const FramebufferSettings& other) const { return !(*this == other); }
  };

  class TK_API Framebuffer
  {
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

    RenderTargetPtr SetColorAttachment(Attachment atc,
                                       RenderTargetPtr rt,
                                       int mip          = 0,
                                       int layer        = -1,
                                       CubemapFace face = CubemapFace::NONE);

    RenderTargetPtr GetColorAttachment(Attachment atc);
    RenderTargetPtr DetachColorAttachment(Attachment atc);
    void ClearAttachments();

    DepthTexturePtr GetDepthTexture();
    void AttachDepthTexture(DepthTexturePtr rt);
    void RemoveDepthAttachment();

    uint GetFboId();
    const FramebufferSettings& GetSettings();
    void ReconstructIfNeeded(int width, int height);
    void ReconstructIfNeeded(const FramebufferSettings& settings);

   private:
    void SetDrawBuffers();
    bool IsColorAttachment(Attachment atc);
    void CheckFramebufferComplete();

   public:
    static const int m_maxColorAttachmentCount = 8;

   private:
    FramebufferSettings m_settings;

    bool m_initialized  = false;
    uint m_fboId        = 0;
    uint m_defaultRboId = 0;
    RenderTargetPtr m_colorAtchs[m_maxColorAttachmentCount];
    DepthTexturePtr m_depthAtch = nullptr;
  };

}; // namespace ToolKit
