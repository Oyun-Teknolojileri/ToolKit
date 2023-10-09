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

#include "BloomPass.h"
#include "CubemapPass.h"
#include "EngineSettings.h"
#include "ForwardPass.h"
#include "ForwardPreProcessPass.h"
#include "FxaaPass.h"
#include "Pass.h"
#include "RenderSystem.h"
#include "ShadowPass.h"
#include "SsaoPass.h"
#include "ToneMapPass.h"
#include "SceneRenderPath.h"

namespace ToolKit
{
  /**
   * Mobile scene render path.
   */
  class TK_API MobileSceneRenderPath : public SceneRenderPath
  {
   public:
    MobileSceneRenderPath();
    explicit MobileSceneRenderPath(const SceneRenderPathParams& params);
    virtual ~MobileSceneRenderPath();

    void Render(Renderer* renderer) override;
    void PreRender(Renderer* renderer) override;
    void PostRender() override;

   private:
    void SetPassParams() override;
  };

  typedef std::shared_ptr<MobileSceneRenderPath> MobileSceneRenderPathPtr;
} // namespace ToolKit
