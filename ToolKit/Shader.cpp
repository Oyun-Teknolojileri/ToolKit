#include "Shader.h"

#include "GL/glew.h"
#include "TKAssert.h"
#include "ToolKit.h"
#include "Util.h"
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include "rapidxml_utils.hpp"

#include <unordered_set>
#include <vector>

#include "DebugNew.h"

namespace ToolKit
{

  Shader::Shader() {}

  Shader::Shader(String file) : Shader() { SetFile(file); }

  Shader::~Shader() { UnInit(); }

  void Shader::Load()
  {
    if (m_loaded)
    {
      return;
    }

    XmlFilePtr file = GetFileManager()->GetXmlFile(GetFile());
    XmlDocument doc;
    doc.parse<rapidxml::parse_full>(file->data());
    if (XmlNode* rootNode = doc.first_node("shader"))
    {
      DeSerialize(&doc, rootNode);
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
    case Uniform::UNUSEDSLOT_1:
      return "UNUSEDSLOT_1";
    case Uniform::EXPOSURE:
      return "Exposure";
    case Uniform::PROJECT_VIEW:
      return "ProjectView";
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

  void Shader::Serialize(XmlDocument* doc, XmlNode* parent) const
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

    for (String file : m_includeFiles)
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
  }

  void Shader::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    m_includeFiles.clear();

    XmlNode* rootNode = parent;
    for (XmlNode* node = rootNode->first_node(); node;
         node          = node->next_sibling())
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
          case Uniform::UNUSEDSLOT_1:
          case Uniform::UNUSEDSLOT_2:
          case Uniform::UNUSEDSLOT_3:
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
  }

  void Shader::SetShaderParameter(String param, const ParameterVariant& val)
  {
    m_shaderParams[param] = val;
  }

  void Shader::UpdateShaderParameters() {}

  void Shader::HandleShaderIncludes(const String& file)
  {
    // Handle source of shader

    ShaderPtr includeShader =
        GetShaderManager()->Create<Shader>(ShaderPath(file, true));
    String includeSource = includeShader->m_source; // Copy

    // Crop the version and precision defines
    size_t versionLoc    = includeSource.find("#version");
    if (versionLoc != String::npos)
    {
      for (size_t fileLoc = versionLoc; fileLoc < includeSource.length();
           ++fileLoc)
      {
        if (includeSource[fileLoc] == '\n')
        {
          includeSource =
              includeSource.replace(versionLoc, fileLoc - versionLoc + 1, "");
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
        for (size_t fileLoc = precisionLoc; fileLoc < includeSource.length();
             ++fileLoc)
        {
          if (includeSource[fileLoc] == ';')
          {
            includeSource = includeSource.replace(precisionLoc,
                                                  fileLoc - precisionLoc + 1,
                                                  "");
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
    while ((precisionLoc = m_source.find("precision", precisionLoc)) !=
           String::npos)
    {
      for (size_t fileLoc = precisionLoc; fileLoc < m_source.length();
           ++fileLoc)
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

  ShaderManager::ShaderManager() { m_type = ResourceType::Shader; }

  ShaderManager::~ShaderManager() {}

  void ShaderManager::Init() { ResourceManager::Init(); }

  bool ShaderManager::CanStore(ResourceType t)
  {
    return t == ResourceType::Shader;
  }

  ResourcePtr ShaderManager::CreateLocal(ResourceType type)
  {
    return ResourcePtr(new Shader());
  }

} // namespace ToolKit
