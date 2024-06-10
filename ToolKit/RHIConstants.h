#pragma once

#include "Types.h"

namespace ToolKit
{
  struct RHIConstants
  {
    static constexpr ubyte TextureSlotCount      = 32;
    static constexpr ubyte MaxLightsPerObject    = 16;
    static constexpr uint ShadowAtlasSlot        = 8;
    static constexpr uint ShadowAtlasTextureSize = 2048;
    static constexpr uint SpecularIBLLods        = 7;
    static constexpr uint BrdfLutTextureSize     = 512;
    static constexpr float ShadowBiasMultiplier  = 0.0001f;
    static constexpr uint LightCacheSize         = 64;
    static constexpr int MaxCascadeCount         = 4;
  };
} // namespace ToolKit