/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
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

    DataTexture();
    virtual void NativeConstruct(int width, int height);

    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;

   protected:
    void Load() override;
    void Clear() override;
  };

  typedef std::shared_ptr<DataTexture> DataTexturePtr;

  class TK_API LightDataTexture : public DataTexture
  {
   public:
    TKDeclareClass(LightDataTexture, DataTexture);

    LightDataTexture();
    using DataTexture::NativeConstruct;

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
    bool IncrementDataIndex(int& index, int amount = 1);
  };

  typedef std::shared_ptr<LightDataTexture> LightDataTexturePtr;

} // namespace ToolKit
