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
#include "Types.h"

namespace ToolKit
{
  class TK_API DataTexture : public Texture
  {
   public:
    TKDeclareClass(DataTexture, Texture);

    DataTexture(int width, int height);

    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;

   protected:
    DataTexture();
    void Load() override;
    void Clear() override;
  };

  class TK_API LightDataTexture : public DataTexture
  {
   public:
    TKDeclareClass(LightDataTexture, DataTexture);

    LightDataTexture(int width, int height);

    void Init(bool flushClientSideArray = false) override;

    void UpdateTextureData(LightPtrArray& lights,
                           Vec2& shadowDirLightIndexInterval,
                           Vec2& shadowPointLightIndexInterval,
                           Vec2& shadowSpotLightIndexInterval,
                           Vec2& nonShadowDirLightIndexInterval,
                           Vec2& nonShadowPointLightIndexInterval,
                           Vec2& nonShadowSpotLightIndexInterval,
                           float& sizeD,
                           float& sizeP,
                           float& sizeS,
                           float& sizeND,
                           float& sizeNP,
                           float& sizeNS);

   private:
    LightDataTexture();
    bool IncrementDataIndex(int& index, int amount = 1);
  };

} // namespace ToolKit
