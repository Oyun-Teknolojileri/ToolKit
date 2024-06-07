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
   * Reference: https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL
   * Layout std140 padding rules:
   * Each scalar component is 4 bytes
   * Each Vec2 is 8 bytes, Vec3 and Vec4 is 16 bytes. (Vec3 == Vec4 in size)
   * Each scaler and vector in an array is a Vec4
   * Struct size is calculated according to above, and whole size must be multiple of Vec4
   */
  struct PerLightData
  {
    /* Light properties */
    int type; //!< Light type which can be { 0: Directional, 1: Point, 2: Spot }
    Vec3 pos;
    float pad1; //!< Padding to complete Vec3 to Vec4
    Vec3 dir;
    float pad2; //!< Padding to complete Vec3 to Vec4
    Vec3 color;
    float pad3; //!< Padding to complete Vec3 to Vec4
    float intensity;
    float radius;
    float outAngle;
    float innAngle;

    /* Cascade */
    Mat4 projectionViewMatrices[RHIConstants::MaxCascadeCount];
    int numOfCascades;

    /* Shadow */
    float shadowMapCameraFar;
    int castShadow;
    float BleedingReduction;
    int softShadows;
    float shadowBias;

    /* Shadow filter */
    int PCFSamples;
    float PCFRadius;

    /* Atlas settings */
    float shadowAtlasResRatio; //!< Shadow map resolution / Shadow atlas resolution. Used to find UV coordinates.
    Vec3 pad4;

    /** i'th cascade's layer in atlas. Only x component of Vec4 is used, rest is padding. */
    Vec4 shadowAtlasLayer[RHIConstants::MaxCascadeCount];

    /** i 'th cascade' s start coordinates in atlas. Only x,y components of Vec4 are used, rest is padding. */
    Vec4 shadowAtlasCoord[RHIConstants::MaxCascadeCount];
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
