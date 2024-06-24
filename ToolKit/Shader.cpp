/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Shader.h"

#include "FileManager.h"
#include "GpuProgram.h"
#include "Logger.h"
#include "TKAssert.h"
#include "TKOpenGL.h"
#include "ToolKit.h"
#include "Util.h"

#include <unordered_set>

namespace ToolKit
{

#define TK_DEFAULT_FORWARD_FRAG  "defaultFragment.shader"
#define TK_DEFAULT_VERTEX_SHADER "defaultVertex.shader"

  TKDefineClass(Shader, Resource);

  Shader::Shader() {}

  Shader::Shader(const String& file) : Shader() { SetFile(file); }

  Shader::~Shader() { UnInit(); }

  void Shader::Load()
  {
    if (!m_loaded)
    {
      ParseDocument("shader", true);
      m_loaded = true;
    }
  }

  void Shader::Init(bool flushClientSideArray)
  {
    if (m_initiated)
    {
      return;
    }

    if (!m_defineArray.empty())
    {
      ShaderDefineCombinaton defineCombo;
      ComplieShaderCombinations(m_defineArray, 0, defineCombo);
    }
    else
    {
      Compile(m_source);
    }

    if (flushClientSideArray)
    {
      m_source.clear();
    }

    m_initiated = true;
  }

  void Shader::UnInit()
  {
    glDeleteShader(m_shaderHandle);
    m_initiated = false;
  }

  void Shader::SetDefine(const String& name, const String& val)
  {
    // Sanity checks.
    if (!m_initiated)
    {
      TK_ERR("Initialize the shader before setting a value for a define.");
      return;
    }

    // Construct the key.
    String key;
    for (int i = 0; i < (int) m_currentDefineValues.size(); i++)
    {
      int defineIndx  = m_currentDefineValues[i].define;
      int variantIndx = m_currentDefineValues[i].variant;

      // If define found
      if (m_defineArray[defineIndx].define == name)
      {
        // find the variant.
        variantIndx = -1;
        for (int ii = 0; ii < (int) m_defineArray[defineIndx].variants.size(); ii++)
        {
          if (m_defineArray[defineIndx].variants[ii] == val)
          {
            variantIndx                      = ii;
            m_currentDefineValues[i].variant = ii; // update current define variant.
            break;
          }
        }

        if (variantIndx == -1)
        {
          TK_WRN("Shader define can't be set. There is no variant: %s for define: %s", name.c_str(), val.c_str());
          return;
        }
      }

      String defName  = m_defineArray[defineIndx].define;
      String defVal   = m_defineArray[defineIndx].variants[variantIndx];
      key            += defName + ":" + defVal + "|";
    }

    key.pop_back();

    // Set the shader variant.
    auto handle = m_shaderVariantMap.find(key);
    if (handle != m_shaderVariantMap.end())
    {
      m_shaderHandle = m_shaderVariantMap[key];
    }
    else
    {
      TK_ERR("Unknown shader combination %s", key);
    }
  }

  XmlNode* Shader::SerializeImp(XmlDocument* doc, XmlNode* parent) const { return nullptr; }

  XmlNode* Shader::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    m_includeFiles.clear();

    XmlNode* rootNode = parent;
    for (XmlNode* node = rootNode->first_node(); node; node = node->next_sibling())
    {
      if (strcmp("type", node->name()) == 0)
      {
        XmlAttribute* attr = node->first_attribute("name");
        if (strcmp("vertexShader", attr->value()) == 0)
        {
          m_shaderType = ShaderType::VertexShader;
        }
        else if (strcmp("fragmentShader", attr->value()) == 0)
        {
          m_shaderType = ShaderType::FragmentShader;
        }
        else if (strcmp("includeShader", attr->value()) == 0)
        {
          m_shaderType = ShaderType::IncludeShader;
        }
        else
        {
          assert(false);
        }
      }

      if (strcmp("include", node->name()) == 0)
      {
        m_includeFiles.push_back(node->first_attribute("name")->value());
      }

      if (strcmp("uniform", node->name()) == 0)
      {
        XmlAttribute* nameAttr = node->first_attribute("name");
        XmlAttribute* sizeAttr = node->first_attribute("size");

        bool isUniformFound    = false;
        for (uint i = 0; i < (uint) Uniform::UNIFORM_MAX_INVALID; i++)
        {
          // Skipping unused variables.
          switch ((Uniform) i)
          {
          case Uniform::UNUSEDSLOT_2:
          case Uniform::UNUSEDSLOT_3:
          case Uniform::UNUSEDSLOT_4:
          case Uniform::UNUSEDSLOT_7:
            isUniformFound = true;
            continue;
          }

          // Find uniform from name
          if (strcmp(GetUniformName((Uniform) i), nameAttr->value()) == 0)
          {
            isUniformFound = true;

            if (sizeAttr != nullptr)
            {
              // Uniform is array
              int size = std::atoi(sizeAttr->value());
              m_arrayUniforms.push_back({(Uniform) i, size});
            }
            else
            {
              m_uniforms.push_back((Uniform) i);
            }

            break;
          }
        }
        assert(isUniformFound);
      }

      if (strcmp("define", node->name()) == 0)
      {
        ShaderDefine def;
        def.define = node->first_attribute("name")->value();

        String val = node->first_attribute("val")->value();
        Split(val, ",", def.variants);

        m_defineArray.push_back(def);
      }

      if (strcmp("source", node->name()) == 0)
      {
        m_source = node->first_node()->value();
      }
    }

    // Iterate back to forth
    for (auto i = m_includeFiles.rbegin(); i != m_includeFiles.rend(); ++i)
    {
      HandleShaderIncludes(*i);
    }

    return nullptr;
  }

  void Shader::HandleShaderIncludes(const String& file)
  {
    uint mergeLoc           = FindShaderMergeLocation(m_source);
    ShaderPtr includeShader = GetShaderManager()->Create<Shader>(ShaderPath(file, true));
    m_source.replace(mergeLoc, 0, includeShader->m_source);

    for (ShaderDefine& def : includeShader->m_defineArray)
    {
      m_defineArray.push_back(def);
    }

    m_defineArray.erase(m_defineArray.end(), std::unique(m_defineArray.begin(), m_defineArray.end()));

    for (Uniform uniform : includeShader->m_uniforms)
    {
      m_uniforms.push_back(uniform);
    }

    m_uniforms.erase(m_uniforms.end(), std::unique(m_uniforms.begin(), m_uniforms.end()));

    for (ArrayUniform uni : includeShader->m_arrayUniforms)
    {
      m_arrayUniforms.push_back(uni);
    }

    m_arrayUniforms.erase(m_arrayUniforms.end(), std::unique(m_arrayUniforms.begin(), m_arrayUniforms.end()));
  }

  uint Shader::FindShaderMergeLocation(const String& file)
  {
    // Put included file after precision and version defines
    size_t includeLoc = 0;
    size_t versionLoc = m_source.find("#version");
    for (size_t fileLoc = versionLoc; fileLoc < m_source.length(); fileLoc++)
    {
      if (m_source[fileLoc] == '\n')
      {
        includeLoc = std::max(includeLoc, fileLoc + 1);
        break;
      }
    }

    size_t precisionLoc = 0;
    while ((precisionLoc = m_source.find("precision", precisionLoc)) != String::npos)
    {
      for (size_t fileLoc = precisionLoc; fileLoc < m_source.length(); fileLoc++)
      {
        if (m_source[fileLoc] == ';')
        {
          includeLoc = std::max(includeLoc, fileLoc + 3);
          break;
        }
      }

      precisionLoc += 9;
    }

    return (uint) includeLoc;
  }

  uint Shader::Compile(String source)
  {
    GLenum type = 0;
    if (m_shaderType == ShaderType::VertexShader)
    {
      type = (GLenum) GraphicTypes::VertexShader;
    }
    else if (m_shaderType == ShaderType::FragmentShader)
    {
      type = (GLenum) GraphicTypes::FragmentShader;
    }
    else
    {
      TK_ERR("Include shader can't be compiled: %s", GetFile().c_str());
      return 0;
    }

    m_shaderHandle = glCreateShader(type);
    if (m_shaderHandle == 0)
    {
      return 0;
    }

    // Start with #version
    const char* str = nullptr;
    size_t loc      = source.find("#version");
    if (loc != String::npos)
    {
      source = source.substr(loc);
      str    = source.c_str();
    }
    else
    {
      str = source.c_str();
    }

    glShaderSource(m_shaderHandle, 1, &str, nullptr);
    glCompileShader(m_shaderHandle);

    GLint compiled;
    glGetShaderiv(m_shaderHandle, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
      GLint infoLen = 0;
      glGetShaderiv(m_shaderHandle, GL_INFO_LOG_LENGTH, &infoLen);
      if (infoLen > 1)
      {
        char* log = new char[infoLen];
        glGetShaderInfoLog(m_shaderHandle, infoLen, nullptr, log);

        TK_ERR(log);
        TK_ERR(str);

        assert(compiled);
        SafeDelArray(log);
      }

      glDeleteShader(m_shaderHandle);
      return 0;
    }

    return m_shaderHandle;
  }

  void Shader::CompileWithDefines(String source, const ShaderDefineCombinaton& defineCombo)
  {
    String key; // Hash key for the shader variant.
    String defineText;
    for (const ShaderDefineIndex& def : defineCombo)
    {
      String defName  = m_defineArray[def.define].define;
      String defVal   = m_defineArray[def.define].variants[def.variant];
      key            += defName + ":" + defVal + "|";

      defineText     += "#define " + defName + " " + defVal + "\n";
    }

    // Insert defines.
    uint mergeLoc = FindShaderMergeLocation(source);
    source.insert(mergeLoc, defineText);

    key.pop_back(); // remove last "|"

    if (Compile(source) != 0)
    {
      m_currentDefineValues   = defineCombo;
      m_shaderVariantMap[key] = m_shaderHandle;
    }
  }

  void Shader::ComplieShaderCombinations(const ShaderDefineArray& defineArray,
                                         int index,
                                         ShaderDefineCombinaton& currentCombinaiton)
  {
    if (index == defineArray.size())
    {
      // Compile the final combo.
      CompileWithDefines(m_source, currentCombinaiton);
      return;
    }

    const ShaderDefine& currentDefine = defineArray[index];
    for (int vi = (int) currentDefine.variants.size() - 1; vi >= 0; vi--)
    {
      currentCombinaiton.push_back({index, vi});

      // Recursively generate combinations
      ComplieShaderCombinations(defineArray, index + 1, currentCombinaiton);
      currentCombinaiton.pop_back();
    }
  }

  // ShaderManager
  //////////////////////////////////////////////////////////////////////////

  ShaderManager::ShaderManager() { m_baseType = Shader::StaticClass(); }

  ShaderManager::~ShaderManager() {}

  void ShaderManager::Init()
  {
    ResourceManager::Init();

    m_pbrForwardShaderFile    = ShaderPath(TK_DEFAULT_FORWARD_FRAG, true);
    m_defaultVertexShaderFile = ShaderPath(TK_DEFAULT_VERTEX_SHADER, true);

    Create<Shader>(m_pbrForwardShaderFile);
    Create<Shader>(m_defaultVertexShaderFile);
  }

  bool ShaderManager::CanStore(ClassMeta* Class) { return Class == Shader::StaticClass(); }

  ShaderPtr ShaderManager::GetDefaultVertexShader() { return Cast<Shader>(m_storage[m_defaultVertexShaderFile]); }

  ShaderPtr ShaderManager::GetPbrForwardShader() { return Cast<Shader>(m_storage[m_pbrForwardShaderFile]); }

  const String& ShaderManager::PbrForwardShaderFile() { return m_pbrForwardShaderFile; }

} // namespace ToolKit
