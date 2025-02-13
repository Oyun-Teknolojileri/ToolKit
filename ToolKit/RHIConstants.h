/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

namespace ToolKit
{

  struct RHIConstants
  {
    static constexpr ubyte TextureSlotCount      = 32;
    static constexpr ubyte MaxLightsPerObject    = 128;
    static constexpr uint ShadowAtlasSlot        = 8;
    static constexpr uint SpecularIBLLods        = 7;
    static constexpr uint BrdfLutTextureSize     = 512;
    static constexpr float ShadowBiasMultiplier  = 0.0001f;
    /** Update lighting.shader MAX_LIGHT_COUNT accordingly. */
    static constexpr uint LightCacheSize         = 40;
    /** Update shadow.shader MAX_CASCADE_COUNT accordingly. */
    static constexpr int MaxCascadeCount         = 4;
    /** Update shadow.shader SHADOW_ATLAS_SIZE accordingly. */
    static constexpr uint ShadowAtlasTextureSize = 2048;
  };

} // namespace ToolKit