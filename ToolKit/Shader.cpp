#include "Shader.h"

#include <vector>

#include "ToolKit.h"
#include "Util.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "rapidxml_print.hpp"
#include "GL/glew.h"
#include "DebugNew.h"

namespace ToolKit
{

  Shader::Shader()
  {
  }

  Shader::Shader(String file)
    : Shader()
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

    XmlFile file = GetFileManager()->GetXmlFile(GetFile());
    XmlDocument doc;
    doc.parse<rapidxml::parse_full>(file.data());
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

    m_shaderHandle = glCreateShader((GLenum)m_shaderType);
    if (m_shaderHandle == 0)
    {
      return;
    }

    // Start with #version
    const char* str = nullptr;
    size_t loc = m_source.find("#version");
    if (loc != String::npos)
    {
      m_source = m_source.substr(loc);
      str = m_source.c_str();
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

        SafeDelArray(log);
      }

      assert(compiled);
      glDeleteShader(m_shaderHandle);
      return;
    }

    if (flushClientSideArray)
    {
      m_source.clear();
    }

    m_tag = std::to_string(m_shaderHandle);
    m_initiated = true;
  }

  void Shader::UnInit()
  {
    glDeleteShader(m_shaderHandle);
    m_initiated = false;
  }

  void Shader::SetShaderParameter(String param, const ParameterVariant& val)
  {
    m_shaderParams[param] = val;
  }

  void Shader::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* container = CreateXmlNode(doc, "shader", parent);
    XmlNode* node = CreateXmlNode(doc, "type", container);

    if (m_shaderType == GraphicTypes::VertexShader)
    {
      WriteAttr(node, doc, "name", "fragmentShader");
    }
    else if (m_shaderType == GraphicTypes::FragmentShader)
    {
      WriteAttr(node, doc, "name", "vertexShader");
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
        case Uniform::GRID_SETTINGS:
        name = "GridData";
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
        default:
        assert(false && "unknown uniform");
        break;
      }

      WriteAttr(node, doc, "name", name);
    }

    XmlNode* src = CreateXmlNode(doc, "source", container);
    XmlNode* srcInput = doc->allocate_node
    (
      rapidxml::node_type::node_comment
    );
    src->append_node(srcInput);
    srcInput->value(doc->allocate_string(m_source.c_str()));
  }

  void Shader::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    XmlNode* rootNode = parent;
    for
    (
      XmlNode* node = rootNode->first_node();
      node;
      node = node->next_sibling()
    )
    {
      if (strcmp("type", node->name()) == 0)
      {
        XmlAttribute* attr = node->first_attribute("name");
        if (strcmp("vertexShader", attr->value()) == 0)
        {
          m_shaderType = GraphicTypes::VertexShader;
        }
        else if (strcmp("fragmentShader", attr->value()) == 0)
        {
          m_shaderType = GraphicTypes::FragmentShader;
        }
        else
        {
          assert(false);
        }
      }

      if (strcmp("uniform", node->name()) == 0)
      {
        XmlAttribute* attr = node->first_attribute();
        if (strcmp("ProjectViewModel", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::PROJECT_MODEL_VIEW);
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
        else if (strcmp("GridData", attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::GRID_SETTINGS);
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

}  // namespace ToolKit
