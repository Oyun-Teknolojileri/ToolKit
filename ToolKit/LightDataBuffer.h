/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "RHIConstants.h"
#include "Types.h"

namespace ToolKit
{

  /**
   * A gpu memory aligned buffer representing a single light.
   *
   * The buffer in the gpu is using layout std140.
   * To see the correct paddings, use render doc and select show paddings for the corresponding buffer.
   */
  struct PerLightData
  {
    /* Light properties */
    Vec3 pos;
    int type; // 16 byte aligned with vec3.

    Vec3 dir;
    float intensity; // 16 byte aligned with vec3.

    Vec3 color;
    float radius; // 16 byte aligned with vec3.

    float outAngle;
    float innAngle;
    float shadowMapCameraFar;
    int numOfCascades; // 16 byte aligned with 4 x 4 bytes.

    /* Cascade */
    Mat4 projectionViewMatrices[RHIConstants::MaxCascadeCount]; // Vec4 per row. 16 byte aligned.

    float BleedingReduction;
    float shadowBias;
    int castShadow;
    int softShadows; // 16 byte aligned with 4 x 4 bytes.

    int PCFSamples;
    float PCFRadius;
    float shadowAtlasResRatio; //!< Shadow map resolution / Shadow atlas resolution. Used to find UV coordinates.
    float pad0;                // 16 byte aligned with 4 x 4 bytes. Padding is needed to 16 byte alignment.

    Vec4 shadowAtlasLayer[RHIConstants::MaxCascadeCount]; // 4 byte + 12 byte padding.

    Vec4 shadowAtlasCoord[RHIConstants::MaxCascadeCount]; // 8 byte + 8 byte padding.
  };

  struct LightData
  {
    Vec4 cascadeDistances;
    PerLightData perLightData[RHIConstants::LightCacheSize];
  };

  struct ActiveLightIndices
  {
    int activeLightIndices[RHIConstants::LightCacheSize];
  };

  class TK_API LightDataBuffer
  {
   public:
    ~LightDataBuffer();

    void Init();
    void Destroy();

    void Update(LightPtr* cachedLights, int size, const LightRawPtrArray& lightsToRender);
    void UpdateLightData(LightPtr* cachedLights, int size);
    void UpdateLightIndices(const LightRawPtrArray& lightsToRender);

   public:
    uint m_lightDataBufferId    = 0;
    uint m_lightIndicesBufferId = 0;
    LightData m_lightData;
    ActiveLightIndices m_activeLightIndices;

   private:
    bool m_initialized = false;
  };
} // namespace ToolKit
