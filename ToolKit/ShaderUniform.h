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

  /**
   * Predefined uniforms. When used in shaders, engine feeds the values with the right frequency.
   */
  enum class Uniform
  {
    // Order is important. Don't change for backward comparable resource files.
    PROJECT_VIEW_MODEL,
    VIEW,
    MODEL,
    INV_TR_MODEL,
    UNUSEDSLOT_6, // LIGHT_DATA
    UNUSEDSLOT_7, // CAM_DATA
    COLOR,
    FRAME_COUNT,
    ELAPSED_TIME,
    EXPOSURE,
    PROJECT_VIEW_NO_TR,
    USE_IBL,
    IBL_INTENSITY,
    IBL_IRRADIANCE,
    DIFFUSE_TEXTURE_IN_USE,
    COLOR_ALPHA,
    UNUSEDSLOT_4, // USE_AO
    IBL_ROTATION,
    UNUSEDSLOT_2, // Lighting Only
    USE_ALPHA_MASK,
    ALPHA_MASK_TRESHOLD,
    UNUSEDSLOT_5, // USE_FORWARD_PATH
    EMISSIVE_TEXTURE_IN_USE,
    EMISSIVE_COLOR,
    UNUSEDSLOT_3, // LIGHTING_TYPE Phong - PBR - Custom
    METALLIC,
    ROUGHNESS,
    METALLIC_ROUGHNESS_TEXTURE_IN_USE,
    NORMAL_MAP_IN_USE,
    IBL_MAX_REFLECTION_LOD,
    SHADOW_DISTANCE,
    CAM_DATA_POS,
    CAM_DATA_DIR,
    CAM_DATA_FAR,
    LIGHT_DATA_TYPE,
    LIGHT_DATA_POS,
    LIGHT_DATA_DIR,
    LIGHT_DATA_COLOR,
    LIGHT_DATA_INTENSITY,
    LIGHT_DATA_RADIUS,
    LIGHT_DATA_OUTANGLE,
    LIGHT_DATA_INNANGLE,
    LIGHT_DATA_PROJVIEWMATRIX,
    LIGHT_DATA_SHADOWMAPCAMFAR,
    LIGHT_DATA_CASTSHADOW,
    LIGHT_DATA_PCFSAMPLES,
    LIGHT_DATA_PCFRADIUS,
    LIGHT_DATA_BLEEDREDUCTION,
    LIGHT_DATA_SOFTSHADOWS,
    LIGHT_DATA_SHADOWATLASLAYER,
    LIGHT_DATA_SHADOWATLASRESRATIO,
    LIGHT_DATA_SHADOWATLASCOORD,
    LIGHT_DATA_SHADOWBIAS,
    LIGHT_DATA_ACTIVECOUNT,
    IS_SKINNED,
    NUM_BONES,
    UNIFORM_MAX_INVALID
  };

  extern const char* GetUniformName(Uniform u);

  // ShaderUniform
  //////////////////////////////////////////////////////////////////////////

  using UniformValue = std::variant<bool, float, int, uint, Vec2, Vec3, Vec4, Mat3, Mat4>;

  class TK_API ShaderUniform
  {
   public:
    enum class UpdateFrequency
    {
      PerDraw,
      PerFrame,
      PerMaterial,
      WhenDirty
    };

    enum class UniformType
    {
      Undefined,
      Bool,
      Float,
      Int,
      UInt,
      Vec2,
      Vec3,
      Vec4,
      Mat3,
      Mat4
    };

   public:
    ShaderUniform();
    ShaderUniform(const String& name, UniformValue value, UpdateFrequency frequency = UpdateFrequency::WhenDirty);
    ShaderUniform(const ShaderUniform& other);
    ShaderUniform(ShaderUniform&& other) noexcept;

    template <typename T>
    T& GetVal()
    {
      return std::get<T>(m_value);
    }

    UniformType GetType();

    ShaderUniform& operator=(const UniformValue& value);
    ShaderUniform& operator=(const ShaderUniform& other);
    ShaderUniform& operator=(ShaderUniform&& other) noexcept;

   public:
    String m_name;
    UpdateFrequency m_updateFrequency;
    UniformValue m_value;
    bool m_isDirty;
  };

} // namespace ToolKit