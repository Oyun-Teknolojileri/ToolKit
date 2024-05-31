/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "ShaderUniform.h"

#include "TKAssert.h"

namespace ToolKit
{

  const char* GetUniformName(Uniform u)
  {
    switch (u)
    {
    case Uniform::PROJECT_VIEW_MODEL:
      return "ProjectViewModel";
    case Uniform::VIEW:
      return "View";
    case Uniform::MODEL:
      return "Model";
    case Uniform::INV_TR_MODEL:
      return "InverseTransModel";
    case Uniform::SHADOW_ATLAS_SIZE:
      return "shadowAtlasSize";
    case Uniform::UNUSEDSLOT_7:
      TK_ASSERT_ONCE(false && "Old asset in use.");
      return "UNUSEDSLOT_7";
    case Uniform::COLOR:
      return "Color";
    case Uniform::FRAME_COUNT:
      return "FrameCount";
    case Uniform::ELAPSED_TIME:
      return "ElapsedTime";
    case Uniform::EXPOSURE:
      return "Exposure";
    case Uniform::PROJECT_VIEW_NO_TR:
      return "ProjectViewNoTr";
    case Uniform::USE_IBL:
      return "UseIbl";
    case Uniform::IBL_INTENSITY:
      return "IblIntensity";
    case Uniform::IBL_IRRADIANCE:
      return "IBLIrradianceMap";
    case Uniform::DIFFUSE_TEXTURE_IN_USE:
      return "DiffuseTextureInUse";
    case Uniform::COLOR_ALPHA:
      return "ColorAlpha";
    case Uniform::UNUSEDSLOT_4:
      TK_ASSERT_ONCE(false && "Old asset in use.");
      return "UNUSEDSLOT_4";
    case Uniform::IBL_ROTATION:
      return "IblRotation";
    case Uniform::UNUSEDSLOT_2:
      TK_ASSERT_ONCE(false);
      return "UNUSEDSLOT_2";
    case Uniform::USE_ALPHA_MASK:
      return "useAlphaMask";
    case Uniform::ALPHA_MASK_TRESHOLD:
      return "alphaMaskTreshold";
    case Uniform::MODEL_VIEW_MATRIX:
      return "modelViewMatrix";
    case Uniform::EMISSIVE_TEXTURE_IN_USE:
      return "emissiveTextureInUse";
    case Uniform::EMISSIVE_COLOR:
      return "emissiveColor";
    case Uniform::UNUSEDSLOT_3:
      TK_ASSERT_ONCE(false && "Old asset in use.");
      return "UNUSEDSLOT_3";
    case Uniform::METALLIC:
      return "metallic";
    case Uniform::ROUGHNESS:
      return "roughness";
    case Uniform::METALLIC_ROUGHNESS_TEXTURE_IN_USE:
      return "metallicRoughnessTextureInUse";
    case Uniform::NORMAL_MAP_IN_USE:
      return "normalMapInUse";
    case Uniform::IBL_MAX_REFLECTION_LOD:
      return "iblMaxReflectionLod";
    case Uniform::SHADOW_DISTANCE:
      return "shadowDistance";
    case Uniform::CAM_DATA_POS:
      return "CamData.pos";
    case Uniform::CAM_DATA_DIR:
      return "CamData.dir";
    case Uniform::CAM_DATA_FAR:
      return "CamData.far";
    case Uniform::LIGHT_DATA_TYPE:
      return "LightData.type";
    case Uniform::LIGHT_DATA_POS:
      return "LightData.pos";
    case Uniform::LIGHT_DATA_DIR:
      return "LightData.dir";
    case Uniform::LIGHT_DATA_COLOR:
      return "LightData.color";
    case Uniform::LIGHT_DATA_INTENSITY:
      return "LightData.intensity";
    case Uniform::LIGHT_DATA_RADIUS:
      return "LightData.radius";
    case Uniform::LIGHT_DATA_OUTANGLE:
      return "LightData.outAngle";
    case Uniform::LIGHT_DATA_INNANGLE:
      return "LightData.innAngle";
    case Uniform::LIGHT_DATA_PROJVIEWMATRIX:
      return "LightData.projectionViewMatrix";
    case Uniform::LIGHT_DATA_SHADOWMAPCAMFAR:
      return "LightData.shadowMapCameraFar";
    case Uniform::LIGHT_DATA_CASTSHADOW:
      return "LightData.castShadow";
    case Uniform::LIGHT_DATA_PCFSAMPLES:
      return "LightData.PCFSamples";
    case Uniform::LIGHT_DATA_PCFRADIUS:
      return "LightData.PCFRadius";
    case Uniform::LIGHT_DATA_BLEEDREDUCTION:
      return "LightData.BleedingReduction";
    case Uniform::LIGHT_DATA_SOFTSHADOWS:
      return "LightData.softShadows";
    case Uniform::LIGHT_DATA_SHADOWATLASLAYER:
      return "LightData.shadowAtlasLayer";
    case Uniform::LIGHT_DATA_SHADOWATLASRESRATIO:
      return "LightData.shadowAtlasResRatio";
    case Uniform::LIGHT_DATA_SHADOWATLASCOORD:
      return "LightData.shadowAtlasCoord";
    case Uniform::LIGHT_DATA_SHADOWBIAS:
      return "LightData.shadowBias";
    case Uniform::LIGHT_DATA_ACTIVECOUNT:
      return "activeCount";
    case Uniform::IS_SKINNED:
      return "isSkinned";
    case Uniform::NUM_BONES:
      return "numBones";
    case Uniform::KEY_FRAME_1:
      return "keyFrame1";
    case Uniform::KEY_FRAME_2:
      return "keyFrame2";
    case Uniform::KEY_FRAME_INT_TIME:
      return "keyFrameIntepolationTime";
    case Uniform::KEY_FRAME_COUNT:
      return "keyFrameCount";
    case Uniform::IS_ANIMATED:
      return "isAnimated";
    case Uniform::BLEND_ANIMATION:
      return "blendAnimation";
    case Uniform::BLEND_FACTOR:
      return "blendFactor";
    case Uniform::BLEND_KEY_FRAME_1:
      return "blendKeyFrame1";
    case Uniform::BLEND_KEY_FRAME_2:
      return "blendKeyFrame2";
    case Uniform::BLEND_KEY_FRAME_INT_TIME:
      return "blendKeyFrameIntepolationTime";
    case Uniform::BLEND_KEY_FRAME_COUNT:
      return "blendKeyFrameCount";
    case Uniform::MODEL_NO_TR:
      return "modelNoTr";
    case Uniform::AO_ENABLED:
      return "aoEnabled";
    case Uniform::ACTIVE_LIGHT_INDICES:
      return "activeLightIndices";
    case Uniform::UNIFORM_MAX_INVALID:
    default:
      return "";
    }
  }

  // ShaderUniform
  //////////////////////////////////////////////////////////////////////////

  ShaderUniform::ShaderUniform() : m_updateFrequency(UpdateFrequency::PerDraw) {}

  ShaderUniform::ShaderUniform(const String& name, UniformValue value, UpdateFrequency frequency)
      : m_updateFrequency(UpdateFrequency::PerDraw)
  {
    m_name            = name;
    m_value           = value;
    m_updateFrequency = frequency;
  }

  ShaderUniform::ShaderUniform(const ShaderUniform& other)
      : m_name(other.m_name), m_updateFrequency(other.m_updateFrequency), m_value(other.m_value)
  {
  }

  ShaderUniform::ShaderUniform(ShaderUniform&& other) noexcept
      : m_name(std::move(other.m_name)), m_updateFrequency(other.m_updateFrequency), m_value(std::move(other.m_value))
  {
  }

  ShaderUniform& ShaderUniform::operator=(const UniformValue& other)
  {
    m_value = other;
    return *this;
  }

  ShaderUniform& ShaderUniform::operator=(const ShaderUniform& other)
  {
    if (this != &other)
    {
      m_name            = other.m_name;
      m_updateFrequency = other.m_updateFrequency;
      m_value           = other.m_value;
    }
    return *this;
  }

  ShaderUniform& ShaderUniform::operator=(ShaderUniform&& other) noexcept
  {
    if (this != &other)
    {
      m_name            = other.m_name;
      m_updateFrequency = other.m_updateFrequency;
      m_value           = other.m_value;
    }
    return *this;
  }

  ShaderUniform::UniformType ShaderUniform::GetType() { return (UniformType) m_value.index(); }

  bool ShaderUniform::operator==(const UniformValue& other) const
  {
    if (m_value.index() != other.index())
    {
      // Different variant types, not equal
      return false;
    }

    switch (m_value.index())
    {
    case 0: // bool
      return std::get<bool>(m_value) == std::get<bool>(other);
    case 1: // float
      return std::get<float>(m_value) == std::get<float>(other);
    case 2: // int
      return std::get<int>(m_value) == std::get<int>(other);
    case 3: // uint
      return std::get<uint>(m_value) == std::get<uint>(other);
    case 4: // glm::vec2
      return glm::all(glm::equal(std::get<glm::vec2>(m_value), std::get<glm::vec2>(other)));
    case 5: // glm::vec3
      return glm::all(glm::equal(std::get<glm::vec3>(m_value), std::get<glm::vec3>(other)));
    case 6: // glm::vec4
      return glm::all(glm::equal(std::get<glm::vec4>(m_value), std::get<glm::vec4>(other)));
    case 7: // glm::mat3
      return glm::all(glm::equal(std::get<glm::mat3>(m_value), std::get<glm::mat3>(other)));
    case 8: // glm::mat4
      return glm::all(glm::equal(std::get<glm::mat4>(m_value), std::get<glm::mat4>(other)));
    default:
      // Unsupported type, not equal
      return false;
    }
  }

  bool ShaderUniform::operator!=(const UniformValue& other) const { return !(*this == other); }

} // namespace ToolKit