#include "LightDataBuffer.h"

#include "DirectionComponent.h"
#include "Light.h"
#include "RHI.h"

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

  void LightDataBuffer::Update(Light** cachedLights, int size, const LightRawPtrArray& lightsToRender)
  {
    UpdateLightData(cachedLights, size);
    UpdateLightIndices(lightsToRender);
  }

  void LightDataBuffer::UpdateLightData(Light** cachedLights, int size)
  {
    for (int i = 0; i < size; ++i)
    {
      if (cachedLights[i] == nullptr)
      {
        continue;
      }

      const Light* currLight = cachedLights[i];

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

      const bool castShadow = currLight->GetCastShadowVal();
      if (castShadow)
      {
        const int PCFSamples                             = currLight->GetPCFSamplesVal();
        m_lightData.perLightData[i].projectionViewMatrix = currLight->m_shadowMapCameraProjectionViewMatrix;
        m_lightData.perLightData[i].shadowMapCameraFar   = currLight->m_shadowMapCameraFar;
        m_lightData.perLightData[i].BleedingReduction    = currLight->GetBleedingReductionVal();
        m_lightData.perLightData[i].PCFSamples           = PCFSamples;
        m_lightData.perLightData[i].PCFRadius            = currLight->GetPCFRadiusVal();
        m_lightData.perLightData[i].softShadows          = PCFSamples > 1;
        m_lightData.perLightData[i].shadowAtlasLayer     = (float) currLight->m_shadowAtlasLayer;
        m_lightData.perLightData[i].shadowAtlasCoord =
            currLight->m_shadowAtlasCoord / (float) RHIConstants::ShadowAtlasTextureSize;
        m_lightData.perLightData[i].shadowAtlasResRatio =
            currLight->GetShadowResVal() / RHIConstants::ShadowAtlasTextureSize;
        m_lightData.perLightData[i].shadowBias = currLight->GetShadowBiasVal() * RHIConstants::ShadowBiasMultiplier;
      }
      m_lightData.perLightData[i].castShadow = (int) castShadow;
    }
  }

  void LightDataBuffer::UpdateLightIndices(const LightRawPtrArray& lightsToRender)
  {
    for (int i = 0; i < lightsToRender.size(); ++i)
    {
      m_activeLightIndices.activeLightIndices[i] = lightsToRender[i]->m_lightCacheIndex;
    }
  }
} // namespace ToolKit
