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

  /** Enumeration for shader types. */
  enum class ShaderType
  {
    VertexShader,   //!< Vertex shader to execute.
    FragmentShader, //!< Fragment shader to execute.
    IncludeShader   //!< Shader file to be included other shader files.
  };

  /** Struct that holds shader definitions and its variants. */
  struct ShaderDefine
  {
    String define;        //!< Name of the define exactly it appears in shader.
    StringArray variants; //!< Value combinations for the given define.

    bool operator==(const ShaderDefine& other) const { return define == other.define; }
  };

  typedef std::vector<ShaderDefine> ShaderDefineArray;

  /**
   * Shader class that holds the shader source and compiled shader.
   * This class is capable of maintaining multiple versions of the same shader compiled for different define values
   * declared in the .shader file.
   * Notes on defines, master shaders and include shaders:
   * Master shaders are the one which contains the shader entry functions on the other hand include shaders can be
   * included many master shader but they don't have an entry point.
   * Include Shaders should not declare defines. The master shaders should declare defines and set their values. Include
   * shaders however can directly use defines declared in master shaders.
   */
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

    /**
     * Sets a value for a define deceleration in the shader file.
     * There must be a value declared in the shader file matching with new val.
     * To set the define values, the shader must be initialized.
     * This function won't add new defines or variants, only sets the existing shader variant.
     */
    void SetDefine(const String& name, const String& val);

    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

   private:
    /** Internally used to concatenate include shader source with master shader file. */
    void HandleShaderIncludes(const String& file);

    /**
     * Internally used to find a location that includes and defines can be placed.
     * Usually returns the first line after #version and precisions defines.
     */
    uint FindShaderMergeLocation(const String& file);

    /**
     * Compiles the given source string.
     * @return shader handle for compiled shader.
     */
    uint Compile(String source);

    /** Internally used structure to point to a define variant. */
    struct ShaderDefineIndex
    {
      int define  = 0; //!< Index to define in define array.
      int variant = 0; //!< Index to variant of the define.
    };

    typedef std::vector<ShaderDefineIndex> ShaderDefineCombinaton;

    /** Update the source file with the given define combinations and compile the shader. */
    void CompileWithDefines(String source, const ShaderDefineCombinaton& defineArray);

    /** Constructs shader define combinations and compiles each combination. */
    void ComplieShaderCombinations(const ShaderDefineArray& defineArray,
                                   int index,
                                   ShaderDefineCombinaton& currentCombination);

   public:
    struct ArrayUniform
    {
      Uniform uniform;
      int size;

      bool operator==(const ArrayUniform& other) const { return uniform == other.uniform && size == other.size; }
    };

    /** Built-in Uniform's that are required for the shader. */
    std::vector<Uniform> m_uniforms;

    /** Built-in Uniform's that are arrays and required for the shader. */
    std::vector<ArrayUniform> m_arrayUniforms;

    /** Type of the shader. */
    ShaderType m_shaderType = ShaderType::VertexShader;

    /** Internal Id that is being used by graphics API. */
    uint m_shaderHandle     = 0;

    /** Include files that this shader needs. */
    StringArray m_includeFiles;

    /** Shader's source file. */
    String m_source;

    /** Shader defines are stored in this array. */
    ShaderDefineArray m_defineArray;

   private:
    /**
     * Shaders may hold multiple defines and multiple variants per define. Which leads to a combination of
     * Shaders based on defines and their values. The key string is constructed from the current define values and
     * points to the version of the compiled shader for the given combination. The look up table is used to find the
     * program for a given define combination in the string. Key format: DefineName:Value|DefineName:Value ...
     */
    std::unordered_map<String, uint> m_shaderVariantMap;

    /** Current define value pairs in an array. */
    ShaderDefineCombinaton m_currentDefineValues;
  };

  class TK_API ShaderManager : public ResourceManager
  {
   public:
    ShaderManager();
    virtual ~ShaderManager();
    void Init() override;
    bool CanStore(ClassMeta* Class) override;

    ShaderPtr GetDefaultVertexShader();
    ShaderPtr GetPbrForwardShader();
    const String& PbrForwardShaderFile();

   private:
    String m_pbrForwardShaderFile;
    String m_defaultVertexShaderFile;
  };

} // namespace ToolKit
