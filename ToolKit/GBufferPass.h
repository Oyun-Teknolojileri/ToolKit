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

#include "Pass.h"

namespace ToolKit
{

  struct GBufferPassParams
  {
    RenderJobArray RendeJobs       = {};
    CameraPtr Camera               = nullptr;
  };

  class TK_API GBufferPass : public RenderPass
  {
   public:
    GBufferPass();
    explicit GBufferPass(const GBufferPassParams& params);
    ~GBufferPass();

    void PreRender() override;
    void PostRender() override;
    void Render() override;
    void InitGBuffers(int width, int height);
    void UnInitGBuffers();

   public:
    FramebufferPtr m_framebuffer           = nullptr;
    RenderTargetPtr m_gPosRt               = nullptr;
    RenderTargetPtr m_gNormalRt            = nullptr;
    RenderTargetPtr m_gColorRt             = nullptr;
    RenderTargetPtr m_gEmissiveRt          = nullptr;
    RenderTargetPtr m_gLinearDepthRt       = nullptr;
    RenderTargetPtr m_gMetallicRoughnessRt = nullptr;
    RenderTargetPtr m_gIblRt               = nullptr;

    int m_width                            = 1024;
    int m_height                           = 1024;

    GBufferPassParams m_params;

   private:
    bool m_initialized            = false;
    MaterialPtr m_gBufferMaterial = nullptr;
  };

  typedef std::shared_ptr<GBufferPass> GBufferPassPtr;

} // namespace ToolKit