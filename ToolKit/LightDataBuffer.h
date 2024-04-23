#pragma once

#include "RHIConstants.h"
#include "Types.h"

namespace ToolKit
{
  struct PerLightData
  {
    int type;
    Vec3 pad1;
    Vec3 pos;
    float pad2;
    Vec3 dir;
    float pad3;
    Vec3 color;
    float intensity;
    float radius;
    float outAngle;
    float innAngle;
    float pad5;

    Mat4 projectionViewMatrix;
    float shadowMapCameraFar;
    int castShadow;
    int PCFSamples;
    float PCFRadius;
    float BleedingReduction;
    int softShadows;
    float shadowAtlasLayer;
    float shadowAtlasResRatio;
    Vec2 shadowAtlasCoord;
    float shadowBias;
    float pad6;
  };

  struct LightData
  {
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

    void Update(LightPtr* cachedLights, int size, const LightPtrArray& lightsToRender);
    void UpdateLightData(LightPtr* cachedLights, int size);
    void UpdateLightIndices(const LightPtrArray& lightsToRender);

   public:
    uint m_lightDataBufferId    = 0;
    uint m_lightIndicesBufferId = 0;
    LightData m_lightData;
    ActiveLightIndices m_activeLightIndices;

   private:
    bool m_initialized = false;
  };
} // namespace ToolKit
