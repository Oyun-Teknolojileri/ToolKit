/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "ParameterBlock.h"
#include "Resource.h"
#include "ResourceManager.h"

namespace ToolKit
{
  enum class ShaderType
  {
    VertexShader,
    FragmentShader,
    IncludeShader
  };

  enum class Uniform
  {
    // Order is important. Don't change for backward comparable resource files.
    PROJECT_MODEL_VIEW,
    VIEW,
    MODEL,
    INV_TR_MODEL,
    LIGHT_DATA, //TODO remove this
    CAM_DATA, //TODO remove this
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
    UNIFORM_MAX_INVALID
  };

  const char* GetUniformName(Uniform u);

  class TK_API Shader : public Resource
  {
   public:
    TKDeclareClass(Shader, Resource);

    Shader();
    explicit Shader(const String& file);
    virtual ~Shader();

    void Load() override;
    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;

    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

    /**
     * Adds a shader parameter to the parameter array with the given name and
     * ParameterVariant. Shader is looked up with the parameter name "param" and
     * its value set as "val".
     * @param param is the name that the parameter is referred in the shader.
     * @param val is the value of the given parameter.
     */
    void SetShaderParameter(const String& param, const ParameterVariant& val);

    /**
     * Renderer calls this function before feeding shader parameters to give
     * custom shaders a chance to update. Derived classes may
     * override this to update their custom parameters.
     */
    virtual void UpdateShaderParameters();

   private:
    void HandleShaderIncludes(const String& file);

   public:
    struct ArrayUniform
    {
      Uniform uniform;
      int size;
    };

    /**
     * Container that holds custom shader parameters.
     */
    std::unordered_map<String, ParameterVariant> m_shaderParams;

    /**
     * Shader hash to look up. Any shader resolving to the same tag can be
     * accessed from the Shader Manager. It helps avoiding creating the same
     * shader multiple times.
     */
    String m_tag;

    /**
     * Type of the shader.
     */
    ShaderType m_shaderType = ShaderType::VertexShader;

    /**
     * Internal Id that is being used by graphics API.
     */
    uint m_shaderHandle     = 0;

    /**
     * Built-in Uniform's that are required for the shader.
     */
    std::vector<Uniform> m_uniforms;

    /**
     * Built-in Uniform's that are arrays and required for the shader.
     */
    std::vector<ArrayUniform> m_arrayUniforms;

    /**
     * Include files that this shader needs.
     */
    StringArray m_includeFiles;

    /**
     * Shader's source file.
     */
    String m_source;
  };

  class TK_API ShaderManager : public ResourceManager
  {
   public:
    ShaderManager();
    virtual ~ShaderManager();
    void Init() override;
    bool CanStore(ClassMeta* Class) override;
    ResourcePtr CreateLocal(ClassMeta* Class) override;

    ShaderPtr GetDefaultVertexShader();
    ShaderPtr GetPbrDefferedShader();
    ShaderPtr GetPbrForwardShader();
    ShaderPtr GetPhongForwardShader();

    const String& PbrDefferedShaderFile();
    const String& PbrForwardShaderFile();

   private:
    String m_pbrDefferedShaderFile;
    String m_pbrForwardShaderFile;
    String m_defaultVertexShaderFile;
    String m_phongForwardShaderFile;
  };

} // namespace ToolKit
