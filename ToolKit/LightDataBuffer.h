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
   * NOTES FOR A GENERIC AND BETTER GPU CACHE
   * The gpu cache can be generalized and every value that we write to gpu preferably should use
   * Uniform Buffer Objects as much as possible including but not limited to Animation data, Material Data, Light data
   * etc ... This approach minimizes the calls to gl... functions which reduces the driver over head significantly. The
   * best way to achieve caching is, creating a template class that stores the data in the required form. This cache
   * must have a version, when its data invalidated it must be updated for the current gpu program. Gpu program must
   * store versions for each cache that they maintain. Invalidated caches will have a different version and gpu programs
   * will be forced to update their version. Generally, LightDataBuffer class fallows the explained approach but it can
   * be generalized and make better in the fallowing updates.
   */

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
    int shadowAtlasLayer;

    int PCFSamples;
    float PCFRadius;
    float shadowAtlasResRatio; //!< Shadow map resolution / Shadow atlas resolution. Used to find UV coordinates.
    float pad0;                // 16 byte aligned with 4 x 4 bytes. Padding is needed to 16 byte alignment.

    Vec2 shadowAtlasCoord;
    Vec2 pad1;
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

    /** Updates the light data in the gpu. */
    void UpdateLightCache(LightPtr* cachedLights, int size);

    /**
     * Update active light indexes in the gpu.
     * Checks the active light indexes with lightsToRender, update the gpu data if a it doesn't match perfectly.
     * if force set to true, it always update active lights.
     * Cache must be up to date, otherwise indexes points to invalidated lights.
     */
    void UpdateActiveLights(const LightRawPtrArray& lightsToRender, bool force = false);

   public:
    uint m_lightDataBufferId    = 0;
    uint m_lightIndicesBufferId = 0;

   private:
    LightData m_lightData;
    ActiveLightIndices m_activeLightIndices;

    bool m_initialized = false;
  };

} // namespace ToolKit
