#include "Shader.h"

#include "GL/glew.h"
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

  Shader::Shader()
  {
  }

  Shader::Shader(String file) : Shader()
  {
    SetFile(file);
  }

  Shader::~Shader()
  {
    UnInit();
  }

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

      String name;
      switch (ui)
      {
      case Uniform::PROJECT_MODEL_VIEW:
        name = "ProjectViewModel";
        break;
      case Uniform::VIEW:
        name = "View";
        break;
      case Uniform::MODEL:
        name = "Model";
        break;
      case Uniform::INV_TR_MODEL:
        name = "InverseTransModel";
        break;
      case Uniform::LIGHT_DATA:
        name = "LightData";
        break;
      case Uniform::CAM_DATA:
        name = "CamData";
        break;
      case Uniform::COLOR:
        name = "Color";
        break;
      case Uniform::FRAME_COUNT:
        name = "FrameCount";
        break;
      case Uniform::UNUSEDSLOT_1:
        break;
      case Uniform::PROJECTION_VIEW_NO_TR:
        name = "ProjectionViewNoTr";
        break;
      case Uniform::USE_IBL:
        name = "UseIbl";
        break;
      case Uniform::IBL_IRRADIANCE:
        name = "IBLIrradianceMap";
        break;
      case Uniform::USE_AO:
        name = "UseAO";
        break;
      case Uniform::DIFFUSE_TEXTURE_IN_USE:
        name = "DiffuseTextureInUse";
        break;
      case Uniform::IBL_ROTATION:
        name = "IblRotation";
        break;
      case Uniform::LIGHTING_ONLY:
        name = "LightingOnly";
        break;
      case Uniform::USE_ALPHA_MASK:
        name = "useAlphaMask";
        break;
      case Uniform::ALPHA_MASK_TRESHOLD:
        name = "alphaMaskTreshold";
        break;
      default:
        assert(false && "unknown uniform");
        break;
      }

      WriteAttr(node, doc, "name", name);
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
        XmlAttribute* attr = node->first_attribute();
        if (strcmp("ProjectViewModel", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::PROJECT_MODEL_VIEW);
        }
        else if (strcmp("View", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::VIEW);
        }
        else if (strcmp("Model", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::MODEL);
        }
        else if (strcmp("InverseTransModel", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::INV_TR_MODEL);
        }
        else if (strcmp("LightData", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::LIGHT_DATA);
        }
        else if (strcmp("CamData", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::CAM_DATA);
        }
        else if (strcmp("Color", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::COLOR);
        }
        else if (strcmp("FrameCount", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::FRAME_COUNT);
        }
        else if (strcmp("Exposure", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::EXPOSURE);
        }
        else if (strcmp("ProjectionViewNoTr", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::PROJECTION_VIEW_NO_TR);
        }
        else if (strcmp("UseIbl", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::USE_IBL);
        }
        else if (strcmp("IblIntensity", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::IBL_INTENSITY);
        }
        else if (strcmp("IBLIrradianceMap", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::IBL_IRRADIANCE);
        }
        else if (strcmp("DiffuseTextureInUse", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::DIFFUSE_TEXTURE_IN_USE);
        }
        else if (strcmp("ColorAlpha", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::COLOR_ALPHA);
        }
        else if (strcmp("UseAO", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::USE_AO);
        }
        else if (strcmp("IblRotation", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::IBL_ROTATION);
        }
        else if (strcmp("LightingOnly", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::LIGHTING_ONLY);
        }
        else if (strcmp("useAlphaMask", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::USE_ALPHA_MASK);
        }
        else if (strcmp("alphaMaskTreshold", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::ALPHA_MASK_TRESHOLD);
        }
        else
        {
          assert(false);
        }
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

  void Shader::UpdateShaderParameters()
  {
  }

  void Shader::HandleShaderIncludes(const String& file)
  {
    // Handle source of shader

    ShaderPtr includeShader =
        GetShaderManager()->Create<Shader>(ShaderPath(file, true));
    String includeSource = includeShader->m_source; // Copy

    // Crop the version and precision defines
    size_t versionLoc = includeSource.find("#version");
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

    size_t precisionLoc = includeSource.find("precision");
    if (precisionLoc != String::npos)
    {
      for (size_t fileLoc = precisionLoc; fileLoc < includeSource.length();
           ++fileLoc)
      {
        if (includeSource[fileLoc] == ';')
        {
          includeSource = includeSource.replace(
              precisionLoc, fileLoc - precisionLoc + 1, "");
          break;
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
    precisionLoc = m_source.find("precision");
    for (size_t fileLoc = precisionLoc; fileLoc < m_source.length(); ++fileLoc)
    {
      if (m_source[fileLoc] == ';')
      {
        includeLoc = std::max(includeLoc, fileLoc + 3);
        break;
      }
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

  Program::Program()
  {
  }

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

  ShaderManager::ShaderManager()
  {
    m_type = ResourceType::Shader;
  }

  ShaderManager::~ShaderManager()
  {
  }

  void ShaderManager::Init()
  {
    ResourceManager::Init();
  }

  bool ShaderManager::CanStore(ResourceType t)
  {
    return t == ResourceType::Shader;
  }

  ResourcePtr ShaderManager::CreateLocal(ResourceType type)
  {
    return ResourcePtr(new Shader());
  }

} // namespace ToolKit
