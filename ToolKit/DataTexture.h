#pragma once

#include "Texture.h"
#include "Types.h"

namespace ToolKit
{
  class TK_API DataTexture : public Texture
  {
   public:
    TKResourceType(DataTexture)

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
    LightDataTexture(int width, int height);

    void Init(bool flushClientSideArray = false) override;

    void UpdateTextureData(LightRawPtrArray& lights,
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

  class SSAONoiseTexture : public DataTexture
  {
   public:
    SSAONoiseTexture(int width, int height);

    void Init(void* data);

   protected:
    SSAONoiseTexture();

   private:
    void Init(bool flushClientSideArray = false) override;
  };

  using SSAONoiseTexturePtr = std::shared_ptr<SSAONoiseTexture>;

} // namespace ToolKit
