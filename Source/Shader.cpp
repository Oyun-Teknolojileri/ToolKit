#include "stdafx.h"
#include "Shader.h"
#include "ToolKit.h"
#include "Util.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "rapidxml_print.hpp"
#include "DebugNew.h"

#include <vector>

namespace ToolKit
{

  Shader::Shader()
  {
    m_type = ResourceType::Shader;
  }

  Shader::Shader(String file)
    : Shader()
  {
    m_file = file;
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

    XmlFile file(m_file.c_str());
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

    m_shaderHandle = glCreateShader(m_shaderType);
    if (m_shaderHandle == 0)
    {
      return;
    }

    const char* str = m_source.c_str();
    glShaderSource(m_shaderHandle, 1, &str, nullptr);
    glCompileShader(m_shaderHandle);

    GLint compiled;
    glGetShaderiv(m_shaderHandle, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
      assert(compiled);
      GLint infoLen = 0;
      glGetShaderiv(m_shaderHandle, GL_INFO_LOG_LENGTH, &infoLen);
      if (infoLen > 1)
      {
        char* log = new char[infoLen];
        glGetShaderInfoLog(m_shaderHandle, infoLen, nullptr, log);
        Logger::GetInstance()->Log(log);

        SafeDelArray(log);
      }

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
    XmlNode* container = doc->allocate_node
    (
      rapidxml::node_type::node_element,
      "shader"
    );

    if (parent != nullptr)
    {
      parent->append_node(container);
    }
    else
    {
      doc->append_node(container);
    }

    XmlNode* node = doc->allocate_node
    (
      rapidxml::node_type::node_element,
      "type"
    );
    container->append_node(node);

    if (m_shaderType == GL_VERTEX_SHADER)
    {
      WriteAttr(node, doc, "name", "fragmentShader");
    }
    else if (m_shaderType == GL_FRAGMENT_SHADER)
    {
      WriteAttr(node, doc, "name", "vertexShader");
    }

    for (Uniform ui : m_uniforms)
    {
      XmlNode* node = doc->allocate_node
      (
        rapidxml::node_type::node_element,
        "uniform"
      );
      container->append_node(node);

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
      default:
        assert(false && "unknown uniform");
        break;
      }

      WriteAttr(node, doc, "name", name);
    }

    XmlNode* src = doc->allocate_node
    (
      rapidxml::node_type::node_element,
      "source"
    );
    container->append_node(src);

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
    for (XmlNode* node = rootNode->first_node(); node; node = node->next_sibling())
    {
      if (String("type").compare(node->name()) == 0)
      {
        XmlAttribute* attr = node->first_attribute("name");
        if (String("vertexShader").compare(attr->value()) == 0)
        {
          m_shaderType = GL_VERTEX_SHADER;
        }
        else if (String("fragmentShader").compare(attr->value()) == 0)
        {
          m_shaderType = GL_FRAGMENT_SHADER;
        }
        else
        {
          assert(false);
        }
      }

      if (String("uniform").compare(node->name()) == 0)
      {
        XmlAttribute* attr = node->first_attribute();
        if (String("ProjectViewModel").compare(attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::PROJECT_MODEL_VIEW);
        }
        else if (String("Model").compare(attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::MODEL);
        }
        else if (String("InverseTransModel").compare(attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::INV_TR_MODEL);
        }
        else if (String("LightData").compare(attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::LIGHT_DATA);
        }
        else if (String("CamData").compare(attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::CAM_DATA);
        }
        else if (String("Color").compare(attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::COLOR);
        }
        else if (String("FrameCount").compare(attr->value()) == 0)
        {
          m_uniforms.push_back(Uniform::FRAME_COUNT);
        }
        else
        {
          assert(false);
        }
      }

      if (String("source").compare(node->name()) == 0)
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

    char skinShaderStr[] =
      "#version 300 es\n"
      "struct Bone"
      "{"
      "   mat4 transform;"
      "   mat4 bindPose;"
      "};"
      "in vec3 vPosition;"
      "in vec3 vNormal;"
      "in vec2 vTexture;"
      "in vec3 vBiTan;"
      "in uvec4 vBones;"
      "in vec4 vWeights;"
      "uniform mat4 ProjectViewModel;"
      "uniform Bone bones[64];"
      "out vec3 v_pos;"
      "out vec3 v_normal;"
      "out vec2 v_texture;"
      "out vec3 v_bitan;"
      "void main()"
      "{"
      "   gl_Position = bones[vBones.x].transform * bones[vBones.x].bindPose * vec4(vPosition, 1.0) * vWeights.x;"
      "   gl_Position += bones[vBones.y].transform * bones[vBones.y].bindPose * vec4(vPosition, 1.0) * vWeights.y;"
      "   gl_Position += bones[vBones.z].transform * bones[vBones.z].bindPose * vec4(vPosition, 1.0) * vWeights.z;"
      "   gl_Position += bones[vBones.w].transform * bones[vBones.w].bindPose * vec4(vPosition, 1.0) * vWeights.w;"
      "   v_pos = gl_Position.xyz;"
      "   gl_Position = ProjectViewModel * gl_Position;"
      "   v_texture = vTexture;"
      "   v_normal = vNormal;"
      "   v_bitan = vBiTan;"
      "}";

    ShaderPtr skin = std::make_shared<Shader>();
    skin->m_source = skinShaderStr;
    skin->m_uniforms.push_back(Uniform::PROJECT_MODEL_VIEW);
    skin->Init();

    m_storage[ShaderPath("defaultSkin.shader")] = skin;
  }

}
