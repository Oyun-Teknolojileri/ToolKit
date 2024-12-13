/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "LightDataBuffer.h"

#include "DirectionComponent.h"
#include "EngineSettings.h"
#include "Light.h"
#include "RHI.h"
#include "ToolKit.h"

namespace ToolKit
{

  LightDataBuffer::~LightDataBuffer() { Destroy(); }

  void LightDataBuffer::Init()
  {
    assert(!m_initialized && "LightDataBuffer is already initialized!");

    glGenBuffers(1, &m_lightDataBufferId);
    RHI::BindUniformBuffer(m_lightDataBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(LightData), NULL, GL_DYNAMIC_COPY);

    glGenBuffers(1, &m_lightIndicesBufferId);
    RHI::BindUniformBuffer(m_lightIndicesBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ActiveLightIndices), NULL, GL_DYNAMIC_COPY);
    RHI::BindUniformBuffer(0);

    m_initialized = true;
  }

  void LightDataBuffer::Destroy()
  {
    if (m_initialized)
    {
      glDeleteBuffers(1, &m_lightDataBufferId);
      glDeleteBuffers(1, &m_lightIndicesBufferId);
    }
  }

  void LightDataBuffer::UpdateLightCache(LightPtr* cachedLights, int size)
  {
    const EngineSettings::GraphicSettings& graphicSettings = GetEngineSettings().Graphics;
    m_lightData.cascadeDistances.x                         = graphicSettings.cascadeDistances[0];
    m_lightData.cascadeDistances.y                         = graphicSettings.cascadeDistances[1];
    m_lightData.cascadeDistances.z                         = graphicSettings.cascadeDistances[2];
    m_lightData.cascadeDistances.w                         = graphicSettings.cascadeDistances[3];

    for (int i = 0; i < size; i++)
    {
      if (cachedLights[i] == nullptr)
      {
        continue;
      }

      const Light* currLight = cachedLights[i].get();

      // Point light uniforms
      if (currLight->GetLightType() == Light::Point)
      {
        const PointLight* pLight              = static_cast<const PointLight*>(currLight);
        m_lightData.perLightData[i].type      = 2;
        m_lightData.perLightData[i].pos       = pLight->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        m_lightData.perLightData[i].color     = pLight->GetColorVal();
        m_lightData.perLightData[i].intensity = pLight->GetIntensityVal();
        m_lightData.perLightData[i].radius    = pLight->GetRadiusVal();
      }
      // Directional light uniforms
      else if (currLight->GetLightType() == Light::Directional)
      {
        const DirectionalLight* dLight        = static_cast<const DirectionalLight*>(currLight);
        m_lightData.perLightData[i].type      = 1;
        m_lightData.perLightData[i].color     = dLight->GetColorVal();
        m_lightData.perLightData[i].intensity = dLight->GetIntensityVal();
        m_lightData.perLightData[i].dir       = dLight->GetComponentFast<DirectionComponent>()->GetDirection();
      }
      // Spot light uniforms
      else if (currLight->GetLightType() == Light::Spot)
      {
        const SpotLight* sLight               = static_cast<const SpotLight*>(currLight);
        m_lightData.perLightData[i].type      = 3;
        m_lightData.perLightData[i].color     = sLight->GetColorVal();
        m_lightData.perLightData[i].intensity = sLight->GetIntensityVal();
        m_lightData.perLightData[i].pos       = sLight->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        m_lightData.perLightData[i].dir       = sLight->GetComponentFast<DirectionComponent>()->GetDirection();
        m_lightData.perLightData[i].radius    = sLight->GetRadiusVal();
        m_lightData.perLightData[i].outAngle  = glm::cos(glm::radians(sLight->GetOuterAngleVal() / 2.0f));
        m_lightData.perLightData[i].innAngle  = glm::cos(glm::radians(sLight->GetInnerAngleVal() / 2.0f));
      }

      bool isShadowCaster = currLight->GetCastShadowVal();

      if (isShadowCaster)
      {
        const int PCFSamples = currLight->GetPCFSamplesVal();
        if (currLight->GetLightType() == Light::LightType::Directional)
        {
          const DirectionalLight* dLight               = static_cast<const DirectionalLight*>(currLight);
          m_lightData.perLightData[i].shadowAtlasLayer = dLight->m_shadowAtlasLayers[0];
          m_lightData.perLightData[i].shadowAtlasCoord = dLight->m_shadowAtlasCoords[0];

          const int cascades                           = graphicSettings.cascadeCount;
          for (int ii = 0; ii < cascades; ii++)
          {
            const Mat4& cascadeMatrix = dLight->m_shadowMapCascadeCameraProjectionViewMatrices[ii];
            m_lightData.perLightData[i].projectionViewMatrices[ii] = cascadeMatrix;
          }

          m_lightData.perLightData[i].numOfCascades = cascades;
        }
        else if (currLight->GetLightType() == Light::LightType::Point)
        {
          // Provide layer.
          m_lightData.perLightData[i].shadowAtlasLayer = currLight->m_shadowAtlasLayers[0];

          // Provide coordinate.
          m_lightData.perLightData[i].shadowAtlasCoord = currLight->m_shadowAtlasCoords[0];
        }
        else
        {
          assert(currLight->GetLightType() == Light::LightType::Spot);

          m_lightData.perLightData[i].projectionViewMatrices[0] = currLight->m_shadowMapCameraProjectionViewMatrix;
          m_lightData.perLightData[i].shadowAtlasLayer          = currLight->m_shadowAtlasLayers[0];
          m_lightData.perLightData[i].shadowAtlasCoord          = currLight->m_shadowAtlasCoords[0];
        }

        m_lightData.perLightData[i].shadowMapCameraFar = currLight->m_shadowCamera->Far();
        m_lightData.perLightData[i].BleedingReduction  = currLight->GetBleedingReductionVal();
        m_lightData.perLightData[i].PCFSamples         = PCFSamples;
        m_lightData.perLightData[i].PCFRadius          = currLight->GetPCFRadiusVal();

        float ratio = currLight->GetShadowResVal().GetValue<float>() / RHIConstants::ShadowAtlasTextureSize;
        m_lightData.perLightData[i].shadowAtlasResRatio = ratio;
        m_lightData.perLightData[i].shadowBias = currLight->GetShadowBiasVal() * RHIConstants::ShadowBiasMultiplier;
      }

      m_lightData.perLightData[i].castShadow = (int) isShadowCaster;
    }

    RHI::BindUniformBuffer(m_lightDataBufferId);
    RHI::BindUniformBufferBase(m_lightDataBufferId, 0);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightData), &m_lightData);
  }

  void LightDataBuffer::UpdateActiveLights(const LightRawPtrArray& lightsToRender, bool force)
  {
    bool updateRequired = force;
    uint loopLimit      = glm::min((uint) lightsToRender.size(), RHIConstants::LightCacheSize);

    if (!force)
    {
      for (uint i = 0; i < loopLimit; i++)
      {
        if (m_activeLightIndices.activeLightIndices[i] != lightsToRender[i]->m_lightCacheIndex)
        {
          updateRequired = true;
          break;
        }
      }
    }

    if (updateRequired)
    {
      for (uint i = 0; i < loopLimit; i++)
      {
        m_activeLightIndices.activeLightIndices[i] = lightsToRender[i]->m_lightCacheIndex;
      }

      RHI::BindUniformBuffer(m_lightIndicesBufferId);
      RHI::BindUniformBufferBase(m_lightIndicesBufferId, 1);
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ActiveLightIndices), &m_activeLightIndices);
    }
  }

} // namespace ToolKit
