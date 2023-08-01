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

#pragma once

#include "Texture.h"

namespace ToolKit
{

  struct FramebufferSettings
  {
    uint width           = 1024;
    uint height          = 1024;
    bool depthStencil    = false;
    bool useDefaultDepth = true;

    bool Compare(const FramebufferSettings& settings);
  };

  class TK_API Framebuffer
  {
    // NOTE: This class does not handle renderbuffer attachments, multi-sampled
    // cubemaps, stencil attachments(including depth_stencil).

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
                                  int mip          = 0,
                                  int layer        = -1,
                                  CubemapFace face = CubemapFace::NONE);
    
    DepthTexturePtr GetDepthTexture();
    void AttachDepthTexture(DepthTexturePtr rt);
    RenderTargetPtr GetAttachment(Attachment atc);
    void ClearAttachments();

    uint GetFboId();
    uint GetDefaultRboId();
    FramebufferSettings GetSettings();
    void CheckFramebufferComplete();

    void ReconstructIfNeeded(uint width, uint height);

    RenderTargetPtr DetachAttachment(Attachment atc);
    void RemoveDepthAttachment();
   private:
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
    DepthTexturePtr m_depthAtch = nullptr;
  };

}; // namespace ToolKit
