/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
    LIGHT_DATA,
    CAM_DATA,
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
    /**
     * Container that holds custom shader parameters.
     */
    TKMap<String, ParameterVariant> m_shaderParams;

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
    bool CanStore(TKClass* Class) override;
    ResourcePtr CreateLocal(TKClass* Class) override;

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
