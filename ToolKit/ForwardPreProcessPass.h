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

#include "ForwardPass.h"
#include "Material.h"
#include "Pass.h"
#include "Texture.h"

namespace ToolKit
{

  class TK_API ForwardPreProcess : public RenderPass
  {
   public:
    ForwardPreProcess();
    ~ForwardPreProcess();

    void InitBuffers(uint width, uint height);
    void Render() override;
    void PreRender() override;
    void PostRender() override;

    private:
    void InitDefaultDepthTexture(int width, int height);

   public:
    ForwardRenderPassParams m_params;
    MaterialPtr m_linearMaterial    = nullptr;
    FramebufferPtr m_framebuffer    = nullptr;

    DepthTexturePtr m_depthTexture  = nullptr; // This is used in case there is no gbuffer
    RenderTargetPtr m_normalRt      = nullptr;
    RenderTargetPtr m_linearDepthRt = nullptr;
  };

  typedef std::shared_ptr<ForwardPreProcess> ForwardPreProcessPassPtr;

} // namespace ToolKit
