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

#include "Shader.h"

#include "FileManager.h"
#include "TKAssert.h"
#include "ToolKit.h"
#include "Util.h"

#include "TKOpenGL.h"

#include <unordered_set>

#include "DebugNew.h"

namespace ToolKit
{

#define TK_DEFAULT_DEFERRED_FRAG "deferredRenderFrag.shader"
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

  const char* GetUniformName(Uniform u)
  {
    switch (u)
    {
    case Uniform::PROJECT_MODEL_VIEW:
      return "ProjectViewModel";
    case Uniform::VIEW:
      return "View";
    case Uniform::MODEL:
      return "Model";
    case Uniform::INV_TR_MODEL:
      return "InverseTransModel";
    case Uniform::LIGHT_DATA:
      return "LightData";
    case Uniform::CAM_DATA:
      return "CamData";
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
    case Uniform::UNUSEDSLOT_5:
      TK_ASSERT_ONCE(false && "Old asset in use.");
      return "UNUSEDSLOT_5";
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
    case Uniform::UNIFORM_MAX_INVALID:
    default:
      return "";
    }
  }

  XmlNode* Shader::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* container = CreateXmlNode(doc, "shader", parent);
    XmlNode* node      = CreateXmlNode(doc, "type", container);

    if (m_shaderType == ShaderType::VertexShader)
    {
      WriteAttr(node, doc, "name", "fragmentShader");
    }
    else if (m_shaderType == ShaderType::FragmentShader)
    {
      WriteAttr(node, doc, "name", "vertexShader");
    }
    else if (m_shaderType == ShaderType::IncludeShader)
    {
      WriteAttr(node, doc, "name", "includeShader");
    }

    for (const String& file : m_includeFiles)
    {
      XmlNode* node = CreateXmlNode(doc, "include", container);
      WriteAttr(node, doc, "name", file);
    }

    for (Uniform ui : m_uniforms)
    {
      XmlNode* node = CreateXmlNode(doc, "uniform", container);

      WriteAttr(node, doc, "name", GetUniformName(ui));
    }

    XmlNode* src      = CreateXmlNode(doc, "source", container);
    XmlNode* srcInput = doc->allocate_node(rapidxml::node_type::node_comment);
    src->append_node(srcInput);
    srcInput->value(doc->allocate_string(m_source.c_str()));

    return container;
  }

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
        XmlAttribute* attr  = node->first_attribute();
        bool isUniformFound = false;
        for (uint i = 0; i < (uint) Uniform::UNIFORM_MAX_INVALID; i++)
        {
          // Skipping unused variables.
          switch ((Uniform) i)
          {
          case Uniform::UNUSEDSLOT_2:
          case Uniform::UNUSEDSLOT_3:
          case Uniform::UNUSEDSLOT_4:
          case Uniform::UNUSEDSLOT_5:
            isUniformFound = true;
            continue;
          }

          if (strcmp(GetUniformName((Uniform) i), attr->value()) == 0)
          {
            m_uniforms.push_back((Uniform) i);
            isUniformFound = true;
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
  }

  Program::Program() {}

  Program::Program(ShaderPtr vertex, ShaderPtr fragment)
  {
    m_shaders.push_back(vertex);
    m_shaders.push_back(fragment);

    m_tag = std::to_string(vertex->m_shaderHandle);
    m_tag += std::to_string(fragment->m_shaderHandle);
  }

  Program::~Program()
  {
    glDeleteProgram(m_handle);
    m_handle = 0;
  }

  ShaderManager::ShaderManager() { m_baseType = Shader::StaticClass(); }

  ShaderManager::~ShaderManager() {}

  void ShaderManager::Init()
  {
    ResourceManager::Init();

    m_pbrDefferedShaderFile   = ShaderPath(TK_DEFAULT_DEFERRED_FRAG, true);
    m_pbrForwardShaderFile    = ShaderPath(TK_DEFAULT_FORWARD_FRAG, true);
    m_defaultVertexShaderFile = ShaderPath(TK_DEFAULT_VERTEX_SHADER, true);

    Create<Shader>(m_pbrDefferedShaderFile);
    Create<Shader>(m_pbrForwardShaderFile);
    Create<Shader>(m_defaultVertexShaderFile);
  }

  bool ShaderManager::CanStore(TKClass* Class) { return Class == Shader::StaticClass(); }

  ResourcePtr ShaderManager::CreateLocal(TKClass* Class)
  {
    if (Class == Shader::StaticClass())
    {
      return MakeNewPtr<Shader>();
    }
    return nullptr;
  }

  ShaderPtr ShaderManager::GetDefaultVertexShader() { return Cast<Shader>(m_storage[m_defaultVertexShaderFile]); }

  ShaderPtr ShaderManager::GetPbrDefferedShader() { return Cast<Shader>(m_storage[m_pbrDefferedShaderFile]); }

  ShaderPtr ShaderManager::GetPbrForwardShader() { return Cast<Shader>(m_storage[m_pbrForwardShaderFile]); }

  const String& ShaderManager::PbrDefferedShaderFile() { return m_pbrDefferedShaderFile; }

  const String& ShaderManager::PbrForwardShaderFile() { return m_pbrForwardShaderFile; }

} // namespace ToolKit
