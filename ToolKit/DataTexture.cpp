/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "DataTexture.h"

#include "DirectionComponent.h"
#include "Light.h"
#include "Renderer.h"
#include "TKOpenGL.h"
#include "TKStats.h"

#include "DebugNew.h"

namespace ToolKit
{
  // DataTexture
  //////////////////////////////////////////////////////////////////////////

  TKDefineClass(DataTexture, Texture);

  DataTexture::DataTexture()
  {
// RGBA32 is the format
    m_formatSize = 16;
  }

  void DataTexture::Load() { assert(false); }

  void DataTexture::Clear() { assert(false); }

  void DataTexture::NativeConstruct(int width, int height)
  {
    Super::NativeConstruct();

    m_width  = width;
    m_height = height;
    m_loaded = true;
  }

  void DataTexture::Init(bool flushClientSideArray)
  {
    if (m_initiated)
    {
      return;
    }

    GLint currId;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currId);

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr);

    TKStats::AddVRAMUsageInBytes(m_width * m_height * m_formatSize);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, currId);

    m_initiated = true;
  }

  void DataTexture::UnInit()
  {
    if (m_initiated)
    {
      glDeleteTextures(1, &m_textureId);

      TKStats::RemoveVRAMUsageInBytes(m_width * m_height * m_formatSize);

      m_initiated = false;
    }
  }

  // LightDataTexture
  //////////////////////////////////////////////////////////////////////////

  TKDefineClass(LightDataTexture, Resource);

  LightDataTexture::LightDataTexture() {}

  bool LightDataTexture::IncrementDataIndex(int& index, int amount)
  {
    int verticalIndex = (index + amount) / m_width;
    index             = (index + amount) % m_width;
    return (bool) verticalIndex;
  }

  void LightDataTexture::Init(bool flushClientSideArray) { DataTexture::Init(flushClientSideArray); }

  void LightDataTexture::UpdateTextureData(LightPtrArray& lights,
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
                                           float& sizeNS)
  {
    // Sort the array in order:
    // 1- Shadow caster directional lights
    // 2- Non shadow caster directional lights
    // 3- Shadow caster point lights
    // 4- Non shadow caster point lights
    // 5- Shadow caster spot lights
    // 6- Non shadow caster spot lights

    auto sortByType = [](const LightPtr l1, const LightPtr l2) -> bool
    { return l1->ComparableType() < l2->ComparableType(); };

    auto sortByShadow = [](const LightPtr l1, const LightPtr l2) -> bool
    {
      bool s1 = l1->GetCastShadowVal();
      bool s2 = l2->GetCastShadowVal();
      return s2 ? false : s1;
    };

    std::stable_sort(lights.begin(), lights.end(), sortByShadow);
    std::stable_sort(lights.begin(), lights.end(), sortByType);

    // These variables needs to be updated if you change any data type in light
    // for light data texture
    const float dirShadowSize      = 16.0f;
    const float pointShadowSize    = 14.0f;
    const float spotShadowSize     = 21.0f;
    const float dirNonShadowSize   = 4.0f;
    const float pointNonShadowSize = 5.0f;
    const float spotNonShadowSize  = 8.0f;

    float minShadowDir             = 0.0f;
    float maxShadowDir             = -dirShadowSize;
    float minShadowPoint           = 0.0f;
    float maxShadowPoint           = -pointShadowSize;
    float minShadowSpot            = 0.0f;
    float maxShadowSpot            = -spotShadowSize;
    float minNonShadowDir          = 0.0f;
    float maxNonShadowDir          = -dirNonShadowSize;
    float minNonShadowPoint        = 0.0f;
    float maxNonShadowPoint        = -pointNonShadowSize;
    float minNonShadowSpot         = 0.0f;
    float maxNonShadowSpot         = -spotNonShadowSize;
    bool firstShadowDir            = true;
    bool firstShadowPoint          = true;
    bool firstShadowSpot           = true;
    bool firstNonShadowDir         = true;
    bool firstNonShadowPoint       = true;
    bool firstNonShadowSpot        = true;

    // Set light data into a texture

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    int xIndex = 0;
    int yIndex = 0;

    // This can be packed with a lot more efficiency. Currently we store each
    // element in 4 slotted memory (rgba).

    float i    = 0.0f;
    for (LightPtr light : lights)
    {
      bool shadow = false;
      if (light->GetCastShadowVal())
      {
        shadow = light->GetCastShadowVal();
      }

      float currentSize = 0.0f;

      // Point light
      if (PointLight* pLight = light->As<PointLight>())
      {
        // Check data texture limits
        int size = shadow ? (int) pointShadowSize : (int) pointNonShadowSize;
        if (xIndex + yIndex * m_width + size >= m_width * m_height)
        {
          break;
        }

        Vec3 color      = pLight->GetColorVal();
        float intensity = pLight->GetIntensityVal();
        Vec3 pos        = pLight->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        float radius    = pLight->GetRadiusVal();

        // type
        float t         = 2.0f;
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &t);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;
        // color
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &color.x);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;
        // intensity
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &intensity);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;
        // position
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &pos.x);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;
        // radius
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &radius);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

        if (shadow)
        {
          // Shadow camera far
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &light->m_shadowMapCameraFar);
          yIndex           = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Shadow atlas coordinates
          const Vec2 coord = light->m_shadowAtlasCoord / (float) Renderer::RHIConstants::ShadowAtlasTextureSize;
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &coord.x);
          yIndex            = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Shadow atlas layer
          const float layer = (float) light->m_shadowAtlasLayer;
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &layer);
          yIndex               = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Shadow atlas resolution ratio
          const float resRatio = light->GetShadowResVal() / Renderer::RHIConstants::ShadowAtlasTextureSize;
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &resRatio);
          yIndex                  = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Soft shadows
          const float softShadows = (float) (light->GetPCFSamplesVal() > 1);
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &softShadows);
          yIndex              = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // PCF samples
          const float samples = (float) light->GetPCFSamplesVal();
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &samples);
          yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // PCF radius
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &light->GetPCFRadiusVal());
          yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Light bleeding reduction
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &light->GetBleedingReductionVal());
          yIndex           = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Shadow bias
          const float bias = light->GetShadowBiasVal() * Renderer::RHIConstants::ShadowBiasMultiplier;
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &bias);
          yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          if (firstShadowPoint)
          {
            minShadowPoint   = FLT_MAX;
            firstShadowPoint = false;
          }
          maxShadowPoint = std::max(maxShadowPoint, i);
          minShadowPoint = std::min(minShadowPoint, i);
          currentSize    = pointShadowSize;
        }
        else
        {
          if (firstNonShadowPoint)
          {
            minNonShadowPoint   = FLT_MAX;
            firstNonShadowPoint = false;
          }
          maxNonShadowPoint = std::max(maxNonShadowPoint, i);
          minNonShadowPoint = std::min(minNonShadowPoint, i);
          currentSize       = pointNonShadowSize;
        }
      }
      // Directional light
      else if (DirectionalLight* dLight = light->As<DirectionalLight>())
      {
        // Check data texture limits
        int size = shadow ? (int) dirShadowSize : (int) dirNonShadowSize;
        if (xIndex + yIndex * m_width + size >= m_width * m_height)
        {
          break;
        }

        Vec3 color      = dLight->GetColorVal();
        float intensity = dLight->GetIntensityVal();
        Vec3 dir        = dLight->GetComponent<DirectionComponent>()->GetDirection();

        // type
        float t         = 1.0f;
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &t);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;
        // color
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &color.x);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;
        // intensity
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &intensity);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;
        // direction
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &dir.x);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

        if (shadow)
        {
          // Projection view matrix
          glTexSubImage2D(GL_TEXTURE_2D,
                          0,
                          xIndex,
                          yIndex,
                          4,
                          1,
                          GL_RGBA,
                          GL_FLOAT,
                          &(light->m_shadowMapCameraProjectionViewMatrix[0][0]));
          yIndex           = IncrementDataIndex(xIndex, 4) ? yIndex + 1 : yIndex;

          // Shadow atlas coordinates
          const Vec2 coord = light->m_shadowAtlasCoord / (float) Renderer::RHIConstants::ShadowAtlasTextureSize;
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &coord.x);
          yIndex            = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Shadow atlas layer
          const float layer = (float) light->m_shadowAtlasLayer;
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &layer);
          yIndex               = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Shadow atlas resolution ratio
          const float resRatio = light->GetShadowResVal() / Renderer::RHIConstants::ShadowAtlasTextureSize;
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &resRatio);
          yIndex                  = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Soft shadows
          const float softShadows = (float) (light->GetPCFSamplesVal() > 1);
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &softShadows);
          yIndex              = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // PCF samples
          const float samples = (float) light->GetPCFSamplesVal();
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &samples);
          yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // PCF radius
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &light->GetPCFRadiusVal());
          yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Light bleeding reduction
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &light->GetBleedingReductionVal());
          yIndex           = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Shadow bias
          const float bias = light->GetShadowBiasVal() * Renderer::RHIConstants::ShadowBiasMultiplier;
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &bias);
          yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          if (firstShadowDir)
          {
            minShadowDir   = FLT_MAX;
            firstShadowDir = false;
          }

          maxShadowDir = std::max(maxShadowDir, i);
          minShadowDir = std::min(minShadowDir, i);
          currentSize  = dirShadowSize;
        }
        else
        {
          if (firstNonShadowDir)
          {
            minNonShadowDir   = FLT_MAX;
            firstNonShadowDir = false;
          }
          maxNonShadowDir = std::max(maxNonShadowDir, i);
          minNonShadowDir = std::min(minNonShadowDir, i);
          currentSize     = dirNonShadowSize;
        }
      }
      // Spot light
      else if (SpotLight* sLight = light->As<SpotLight>())
      {
        // Check data texture limits
        int size = shadow ? (int) spotShadowSize : (int) spotNonShadowSize;
        if (xIndex + yIndex * m_width + size >= m_width * m_height)
        {
          break;
        }

        Vec3 color      = sLight->GetColorVal();
        float intensity = sLight->GetIntensityVal();
        Vec3 pos        = sLight->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        Vec3 dir        = sLight->GetComponent<DirectionComponent>()->GetDirection();
        float radius    = sLight->GetRadiusVal();
        float outAngle  = glm::cos(glm::radians(sLight->GetOuterAngleVal() / 2.0f));
        float innAngle  = glm::cos(glm::radians(sLight->GetInnerAngleVal() / 2.0f));

        // type
        float t         = 3.0f;
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &t);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;
        // color
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &color.x);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;
        // intensity
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &intensity);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;
        // position
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &pos.x);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;
        // direction
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &dir.x);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;
        // radius
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &radius);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;
        // outer angle
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &outAngle);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;
        // inner angle
        glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &innAngle);
        yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

        if (shadow)
        {
          // Projection view matrix
          glTexSubImage2D(GL_TEXTURE_2D,
                          0,
                          xIndex,
                          yIndex,
                          4,
                          1,
                          GL_RGBA,
                          GL_FLOAT,
                          &(light->m_shadowMapCameraProjectionViewMatrix[0][0]));
          yIndex = IncrementDataIndex(xIndex, 4) ? yIndex + 1 : yIndex;

          // Shadow camera far
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &light->m_shadowMapCameraFar);
          yIndex           = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Shadow atlas coordinates
          const Vec2 coord = light->m_shadowAtlasCoord / (float) Renderer::RHIConstants::ShadowAtlasTextureSize;
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &coord.x);
          yIndex            = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Shadow atlas layer
          const float layer = (float) light->m_shadowAtlasLayer;
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &layer);
          yIndex               = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Shadow atlas resolution ratio
          const float resRatio = light->GetShadowResVal() / Renderer::RHIConstants::ShadowAtlasTextureSize;
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &resRatio);
          yIndex                  = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Soft shadows
          const float softShadows = (float) (light->GetPCFSamplesVal() > 1);
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &softShadows);
          yIndex              = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // PCF samples
          const float samples = (float) light->GetPCFSamplesVal();
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &samples);
          yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // PCF radius
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &light->GetPCFRadiusVal());
          yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Light bleeding reduction
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &light->GetBleedingReductionVal());
          yIndex           = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          // Shadow bias
          const float bias = light->GetShadowBiasVal() * Renderer::RHIConstants::ShadowBiasMultiplier;
          glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, 1, 1, GL_RGBA, GL_FLOAT, &bias);
          yIndex = IncrementDataIndex(xIndex) ? yIndex + 1 : yIndex;

          if (firstShadowSpot)
          {
            minShadowSpot   = FLT_MAX;
            firstShadowSpot = false;
          }
          maxShadowSpot = std::max(maxShadowSpot, i);
          minShadowSpot = std::min(minShadowSpot, i);
          currentSize   = spotShadowSize;
        }
        else
        {
          if (firstNonShadowSpot)
          {
            minNonShadowSpot   = FLT_MAX;
            firstNonShadowSpot = false;
          }
          maxNonShadowSpot = std::max(maxNonShadowSpot, i);
          minNonShadowSpot = std::min(minNonShadowSpot, i);
          currentSize      = spotNonShadowSize;
        }
      }
      i += currentSize;
    }

    shadowDirLightIndexInterval.x      = minShadowDir;
    shadowDirLightIndexInterval.y      = maxShadowDir + dirShadowSize;
    shadowPointLightIndexInterval.x    = minShadowPoint;
    shadowPointLightIndexInterval.y    = maxShadowPoint + pointShadowSize;
    shadowSpotLightIndexInterval.x     = minShadowSpot;
    shadowSpotLightIndexInterval.y     = maxShadowSpot + spotShadowSize;
    nonShadowDirLightIndexInterval.x   = minNonShadowDir;
    nonShadowDirLightIndexInterval.y   = maxNonShadowDir + dirNonShadowSize;
    nonShadowPointLightIndexInterval.x = minNonShadowPoint;
    nonShadowPointLightIndexInterval.y = maxNonShadowPoint + pointNonShadowSize;
    nonShadowSpotLightIndexInterval.x  = minNonShadowSpot;
    nonShadowSpotLightIndexInterval.y  = maxNonShadowSpot + spotNonShadowSize;
    sizeD                              = dirShadowSize;
    sizeP                              = pointShadowSize;
    sizeS                              = spotShadowSize;
    sizeND                             = dirNonShadowSize;
    sizeNP                             = pointNonShadowSize;
    sizeNS                             = spotNonShadowSize;
  }

} // namespace ToolKit