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

#include "DebugNew.h"

namespace ToolKit
{

#define TK_DEFAULT_DEFERRED_FRAG         "deferredRenderFrag.shader"
#define TK_DEFAULT_FORWARD_FRAG          "defaultFragment.shader"
#define TK_DEFAULT_VERTEX_SHADER         "defaultVertex.shader"
#define TK_PHONG_FORWARD_FRAGMENT_SHADER "phongForwardFragment.shader"

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
      assert(false && "This type should not be compiled.");
    }

    m_shaderHandle = glCreateShader(type);
    if (m_shaderHandle == 0)
    {
      return;
    }

    // Start with #version
    const char* str = nullptr;
    size_t loc      = m_source.find("#version");
    if (loc != String::npos)
    {
      m_source = m_source.substr(loc);
      str      = m_source.c_str();
    }
    else
    {
      str = m_source.c_str();
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
        GetLogger()->WritePlatformConsole(LogType::Error, log);
        GetLogger()->Log(log);
        GetLogger()->Log(GetFile());
        GetLogger()->Log(str);

        assert(compiled);
        SafeDelArray(log);
      }

      glDeleteShader(m_shaderHandle);
      return;
    }

    if (flushClientSideArray)
    {
      m_source.clear();
    }

    m_tag       = std::to_string(m_shaderHandle);
    m_initiated = true;
  }

  void Shader::UnInit()
  {
    glDeleteShader(m_shaderHandle);
    m_initiated = false;
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
          case Uniform::UNUSEDSLOT_5:
          case Uniform::UNUSEDSLOT_6:
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

  void Shader::SetShaderParameter(const String& param, const ParameterVariant& val) { m_shaderParams[param] = val; }

  void Shader::UpdateShaderParameters() {}

  void Shader::HandleShaderIncludes(const String& file)
  {
    // Handle source of shader

    ShaderPtr includeShader = GetShaderManager()->Create<Shader>(ShaderPath(file, true));
    String includeSource    = includeShader->m_source; // Copy

    // Crop the version and precision defines
    size_t versionLoc       = includeSource.find("#version");
    if (versionLoc != String::npos)
    {
      for (size_t fileLoc = versionLoc; fileLoc < includeSource.length(); ++fileLoc)
      {
        if (includeSource[fileLoc] == '\n')
        {
          includeSource = includeSource.replace(versionLoc, fileLoc - versionLoc + 1, "");
          break;
        }
      }
    }

    size_t precisionLoc = 0;
    while (true)
    {
      precisionLoc = includeSource.find("precision");

      if (precisionLoc == String::npos)
      {
        break;
      }

      if (precisionLoc != String::npos)
      {
        for (size_t fileLoc = precisionLoc; fileLoc < includeSource.length(); ++fileLoc)
        {
          if (includeSource[fileLoc] == ';')
          {
            includeSource = includeSource.replace(precisionLoc, fileLoc - precisionLoc + 1, "");
            break;
          }
        }
      }
    }

    // Put included file after precision and version defines
    size_t includeLoc = 0;
    versionLoc        = m_source.find("#version");
    for (size_t fileLoc = versionLoc; fileLoc < m_source.length(); ++fileLoc)
    {
      if (m_source[fileLoc] == '\n')
      {
        includeLoc = std::max(includeLoc, fileLoc + 1);
        break;
      }
    }

    precisionLoc = 0;
    while ((precisionLoc = m_source.find("precision", precisionLoc)) != String::npos)
    {
      for (size_t fileLoc = precisionLoc; fileLoc < m_source.length(); ++fileLoc)
      {
        if (m_source[fileLoc] == ';')
        {
          includeLoc = std::max(includeLoc, fileLoc + 3);
          break;
        }
      }

      precisionLoc += 9;
    }

    m_source.replace(includeLoc, 0, includeSource);

    // Handle uniforms
    std::unordered_set<Uniform> unis;

    for (Uniform uni : m_uniforms)
    {
      unis.insert(uni);
    }
    for (Uniform uni : includeShader->m_uniforms)
    {
      unis.insert(uni);
    }

    m_uniforms.clear();
    for (auto i = unis.begin(); i != unis.end(); ++i)
    {
      m_uniforms.push_back(*i);
    }

    for (ArrayUniform uni : includeShader->m_arrayUniforms)
    {
      m_arrayUniforms.push_back(uni);
    }

    auto arrayUniformCompareFn = [](ArrayUniform& uni1, ArrayUniform& uni2) { return uni1.uniform < uni2.uniform; };

    // Remove duplicates
    std::sort(m_arrayUniforms.begin(), m_arrayUniforms.end(), arrayUniformCompareFn);
    auto uniqueEnd = std::unique(m_arrayUniforms.begin(), m_arrayUniforms.end());
    m_arrayUniforms.erase(uniqueEnd, m_arrayUniforms.end());
  }

  // ShaderManager
  //////////////////////////////////////////////////////////////////////////

  ShaderManager::ShaderManager() { m_baseType = Shader::StaticClass(); }

  ShaderManager::~ShaderManager() {}

  void ShaderManager::Init()
  {
    ResourceManager::Init();

    m_pbrDefferedShaderFile   = ShaderPath(TK_DEFAULT_DEFERRED_FRAG, true);
    m_pbrForwardShaderFile    = ShaderPath(TK_DEFAULT_FORWARD_FRAG, true);
    m_defaultVertexShaderFile = ShaderPath(TK_DEFAULT_VERTEX_SHADER, true);
    m_phongForwardShaderFile  = ShaderPath(TK_PHONG_FORWARD_FRAGMENT_SHADER, true);

    Create<Shader>(m_pbrDefferedShaderFile);
    Create<Shader>(m_pbrForwardShaderFile);
    Create<Shader>(m_defaultVertexShaderFile);
    Create<Shader>(m_phongForwardShaderFile);
  }

  bool ShaderManager::CanStore(ClassMeta* Class) { return Class == Shader::StaticClass(); }

  ShaderPtr ShaderManager::GetDefaultVertexShader() { return Cast<Shader>(m_storage[m_defaultVertexShaderFile]); }

  ShaderPtr ShaderManager::GetPbrDefferedShader() { return Cast<Shader>(m_storage[m_pbrDefferedShaderFile]); }

  ShaderPtr ShaderManager::GetPbrForwardShader() { return Cast<Shader>(m_storage[m_pbrForwardShaderFile]); }

  ShaderPtr ShaderManager::GetPhongForwardShader() { return Cast<Shader>(m_storage[m_phongForwardShaderFile]); }

  const String& ShaderManager::PbrDefferedShaderFile() { return m_pbrDefferedShaderFile; }

  const String& ShaderManager::PbrForwardShaderFile() { return m_pbrForwardShaderFile; }

} // namespace ToolKit
