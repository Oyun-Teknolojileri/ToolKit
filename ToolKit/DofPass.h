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

#include "PostProcessPass.h"

namespace ToolKit
{

  enum class DoFQuality
  {
    Low,    // Radius Scale = 2.0f
    Normal, // Radius Scale = 0.8f
    High    // Radius Scale = 0.2f
  };

  struct DoFPassParams
  {
    RenderTargetPtr ColorRt = nullptr;
    RenderTargetPtr DepthRt = nullptr;
    float focusPoint        = 0.0f;
    float focusScale        = 0.0f;
    DoFQuality blurQuality  = DoFQuality::Normal;
  };

  class TK_API DoFPass : public Pass
  {
   public:
    DoFPass();
    explicit DoFPass(const DoFPassParams& params);
    ~DoFPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    DoFPassParams m_params;

   private:
    FullQuadPassPtr m_quadPass = nullptr;
    ShaderPtr m_dofShader      = nullptr;
  };

  typedef std::shared_ptr<DoFPass> DoFPassPtr;

} // namespace ToolKit