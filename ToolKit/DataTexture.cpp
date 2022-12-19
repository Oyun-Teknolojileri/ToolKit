#include "DataTexture.h"

#include "DirectionComponent.h"
#include "GL/glew.h"
#include "Light.h"

namespace ToolKit
{
  DataTexture::DataTexture()
  {
  }

  void DataTexture::Load()
  {
    assert(false);
  }

  void DataTexture::Clear()
  {
    assert(false);
  }

  DataTexture::DataTexture(int width, int height)
  {
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
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA32F,
                 m_width,
                 m_height,
                 0,
                 GL_RGBA,
                 GL_FLOAT,
                 nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, currId);

    m_initiated = true;
  }

  void DataTexture::UnInit()
  {
    glDeleteTextures(1, &m_textureId);
    m_initiated = false;
  }

  LightDataTexture::LightDataTexture()
  {
  }

  LightDataTexture::LightDataTexture(int width, int height)
      : DataTexture(width, height)
  {
  }

  void LightDataTexture::Init(bool flushClientSideArray)
  {
    DataTexture::Init(flushClientSideArray);
  }

  void LightDataTexture::UnInit()
  {
    DataTexture::UnInit();
  }

  void LightDataTexture::UpdateTextureData(
      LightRawPtrArray& lights,
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

    auto sortByType = [](const Light* l1, const Light* l2) -> bool {
      EntityType t1 = l1->GetType();
      EntityType t2 = l2->GetType();

      if (t1 == EntityType::Entity_DirectionalLight)
      {
        if (t2 == EntityType::Entity_DirectionalLight)
        {
          return false;
        }
        else
        {
          return true;
        }
      }
      else if (t1 == EntityType::Entity_PointLight)
      {
        if (t2 == EntityType::Entity_DirectionalLight ||
            t2 == EntityType::Entity_PointLight)
        {
          return false;
        }
        else
        {
          return true;
        }
      }
      else if (t1 == EntityType::Entity_SpotLight)
      {
        return false;
      }
      else
      {
        return false; // Never comes here
      }
    };

    auto sortByShadow = [](const Light* l1, const Light* l2) -> bool {
      bool s1 = l1->GetCastShadowVal();
      bool s2 = l2->GetCastShadowVal();

      if (s2)
      {
        return false;
      }
      else
      {
        return s1;
      }
    };

    std::stable_sort(lights.begin(), lights.end(), sortByShadow);
    std::stable_sort(lights.begin(), lights.end(), sortByType);

    // TODO These can be member variables
    // These variables needs to be updated if you change any data type in light
    // for light data texture
    const float dirShadowSize      = 4.0f;
    const float pointShadowSize    = 5.0f;
    const float spotShadowSize     = 8.0f;
    const float dirNonShadowSize   = 4.0f;
    const float pointNonShadowSize = 5.0f;
    const float spotNonShadowSize  = 8.0f;

    float minShadowDir       = 0.0f;
    float maxShadowDir       = -dirShadowSize;
    float minShadowPoint     = 0.0f;
    float maxShadowPoint     = -pointShadowSize;
    float minShadowSpot      = 0.0f;
    float maxShadowSpot      = -spotShadowSize;
    float minNonShadowDir    = 0.0f;
    float maxNonShadowDir    = -dirNonShadowSize;
    float minNonShadowPoint  = 0.0f;
    float maxNonShadowPoint  = -pointNonShadowSize;
    float minNonShadowSpot   = 0.0f;
    float maxNonShadowSpot   = -spotNonShadowSize;
    bool firstShadowDir      = true;
    bool firstShadowPoint    = true;
    bool firstShadowSpot     = true;
    bool firstNonShadowDir   = true;
    bool firstNonShadowPoint = true;
    bool firstNonShadowSpot  = true;

    // Set light data into a texture

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    uint index = 0;

    // This can be packed with a lot more efficiency. Currently we store each
    // element in 4 slotted memory (rgba).

    float i = 0.0f;
    for (Light* light : lights)
    {
      bool shadow = false;
      if (light->GetCastShadowVal())
      {
        shadow = light->GetCastShadowVal();
      }

      EntityType type   = light->GetType();
      float currentSize = 0.0f;

      // Point light
      if (type == EntityType::Entity_PointLight)
      {
        Vec3 color      = light->GetColorVal();
        float intensity = light->GetIntensityVal();
        Vec3 pos = light->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        float radius = static_cast<PointLight*>(light)->GetRadiusVal();

        // type
        float t = 2.0;
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &t);
        index += 1;
        // color
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &color.x);
        index += 1;
        // intensity
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &intensity);
        index += 1;
        // position
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &pos.x);
        index += 1;
        // radius
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &radius);
        index += 1;

        if (shadow)
        {
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
      else if (type == EntityType::Entity_DirectionalLight)
      {
        Vec3 color      = light->GetColorVal();
        float intensity = light->GetIntensityVal();
        Vec3 dir        = static_cast<DirectionalLight*>(light)
                       ->GetComponent<DirectionComponent>()
                       ->GetDirection();

        // type
        float t = 1.0;
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &t);
        index += 1;
        // color
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &color.x);
        index += 1;
        // intensity
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &intensity);
        index += 1;
        // direction
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &dir.x);
        index += 1;

        if (shadow)
        {
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
      else if (type == EntityType::Entity_SpotLight)
      {
        Vec3 color      = light->GetColorVal();
        float intensity = light->GetIntensityVal();
        Vec3 pos = light->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        SpotLight* spotLight = static_cast<SpotLight*>(light);
        Vec3 dir =
            spotLight->GetComponent<DirectionComponent>()->GetDirection();
        float radius = spotLight->GetRadiusVal();
        float outAngle =
            glm::cos(glm::radians(spotLight->GetOuterAngleVal() / 2.0f));
        float innAngle =
            glm::cos(glm::radians(spotLight->GetInnerAngleVal() / 2.0f));

        // type
        float t = 3.0;
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &t);
        index += 1;
        // color
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &color.x);
        index += 1;
        // intensity
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &intensity);
        index += 1;
        // position
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &pos.x);
        index += 1;
        // direction
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &dir.x);
        index += 1;
        // radius
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &radius);
        index += 1;
        // outer angle
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &outAngle);
        index += 1;
        // inner angle
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, index, 0, 1, 1, GL_RGBA, GL_FLOAT, &innAngle);
        index += 1;

        if (shadow)
        {
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