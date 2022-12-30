#pragma once

#include "ParameterBlock.h"
#include "Resource.h"
#include "ResourceManager.h"

#include <unordered_map>
#include <vector>

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
    LIGHT_DATA,
    CAM_DATA,
    COLOR,
    FRAME_COUNT,
    UNUSEDSLOT_1,
    EXPOSURE,
    PROJECTION_VIEW_NO_TR,
    USE_IBL,
    IBL_INTENSITY,
    IBL_IRRADIANCE,
    DIFFUSE_TEXTURE_IN_USE,
    COLOR_ALPHA,
    USE_AO,
    IBL_ROTATION,
    UNUSEDSLOT_2, //Lighting Only
    USE_ALPHA_MASK,
    ALPHA_MASK_TRESHOLD,
    USE_FORWARD_PATH,
    EMISSIVE_TEXTURE_IN_USE,
    EMISSIVE_COLOR,
    LIGHTING_TYPE,
    METALLIC,
    ROUGHNESS,
    METALLIC_ROUGHNESS_TEXTURE_UN_USE,
    UNIFORM_MAX_INVALID
  };

  const char* GetUniformName(Uniform u);

  class TK_API Shader : public Resource
  {
   public:
    TKResourceType(Shader)

    Shader();
    explicit Shader(String file);
    virtual ~Shader();

    void Load() override;
    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;

    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    /**
     * Adds a shader parameter to the parameter array with the given name and
     * ParameterVariant. Shader is looked up with the parameter name "param" and
     * its value set as "val".
     * @param param is the name that the parameter is referred in the shader.
     * @param val is the value of the given parameter.
     */
    void SetShaderParameter(String param, const ParameterVariant& val);

    /**
     * Renderer calls this function before feeding shader parameters to give
     * custom shaders a chance to update. Derived classes may
     * override this to update their custom parameters.
     */
    virtual void UpdateShaderParameters();

   private:
    void HandleShaderIncludes(const String& file);

   public:
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
     * Built-in Uniform's that is required for the shader.
     */
    std::vector<Uniform> m_uniforms;

    /**
     * Include files that this shader needs.
     */
    StringArray m_includeFiles;

    /**
     * Shader's source file.
     */
    String m_source;
  };

  class TK_API Program
  {
   public:
    Program();
    Program(ShaderPtr vertex, ShaderPtr fragment);
    ~Program();

   public:
    uint m_handle = 0;
    String m_tag;
    ShaderPtrArray m_shaders;
  };

  class TK_API ShaderManager : public ResourceManager
  {
   public:
    ShaderManager();
    virtual ~ShaderManager();
    void Init() override;
    bool CanStore(ResourceType t) override;
    ResourcePtr CreateLocal(ResourceType type) override;
  };

} // namespace ToolKit
