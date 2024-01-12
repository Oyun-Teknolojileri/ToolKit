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
#include "ShaderUniform.h"

namespace ToolKit
{

  enum class ShaderType
  {
    VertexShader,
    FragmentShader,
    IncludeShader
  };

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
     * Updates or adds the given shader uniform to the uniform cache of the shader.
     */
    void UpdateShaderUniform(const String& name, const UniformValue& val);

    /**
     * Renderer calls this function before feeding shader parameters to give
     * custom shaders a chance to update. Derived classes may
     * override this to update their custom parameters.
     */
    virtual void UpdateShaderUniforms();

   private:
    void HandleShaderIncludes(const String& file);

   public:
    struct ArrayUniform
    {
      Uniform uniform;
      int size;

      bool operator==(const ArrayUniform& other) const { return uniform == other.uniform && size == other.size; }
    };

    /**
     * Built-in Uniform's that are required for the shader.
     */
    std::vector<Uniform> m_uniforms;

    /**
     * Built-in Uniform's that are arrays and required for the shader.
     */
    std::vector<ArrayUniform> m_arrayUniforms;

    /**
     * Container that holds custom shader parameters.
     */
    std::unordered_map<String, ShaderUniform> m_shaderParams;

    /**
     * Type of the shader.
     */
    ShaderType m_shaderType = ShaderType::VertexShader;

    /**
     * Internal Id that is being used by graphics API.
     */
    uint m_shaderHandle     = 0;

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
