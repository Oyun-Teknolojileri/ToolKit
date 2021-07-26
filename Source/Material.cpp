#include "stdafx.h"
#include "Material.h"
#include "ToolKit.h"
#include "Util.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "DebugNew.h"

namespace ToolKit
{

  Material::Material()
  {
    m_color = Vec3(1.0f);
    m_type = ResourceType::Material;
  }

  Material::Material(String file)
    : Material()
  {
    m_file = file;
  }

  Material::~Material()
  {
    UnInit();
  }

  void Material::Load()
  {
    if (m_loaded)
    {
      return;
    }

    XmlFile file(m_file.c_str());
    XmlDocument doc;
    doc.parse<0>(file.data());
    
    XmlNode* rootNode = doc.first_node("material");
    DeSerialize(&doc, rootNode);

    m_loaded = true;
  }

  void Material::Init(bool flushClientSideArray)
  {
    if (m_initiated)
    {
      return;
    }

    if (m_diffuseTexture)
    {
      m_diffuseTexture->Init(flushClientSideArray);
      m_renderState.diffuseTexture = m_diffuseTexture->m_textureId;
      m_renderState.diffuseTextureInUse = true;
    }

    if (m_cubeMap)
    {
      m_cubeMap->Init(flushClientSideArray);
      m_renderState.cubeMap = m_cubeMap->m_textureId;
      m_renderState.cubeMapInUse = true;
    }

    if (m_vertexShader)
    {
      m_vertexShader->Init();
    }
    else
    {
      m_vertexShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultVertex.shader"));
      m_vertexShader->Init();
    }

    if (m_fragmetShader)
    {
      m_fragmetShader->Init();
    }
    else
    {
      if (m_diffuseTexture)
      {
        m_fragmetShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultFragment.shader"));
      }
      else
      {
        m_fragmetShader = GetShaderManager()->Create<Shader>(ShaderPath("solidColorFrag.shader"));
      }
      
      m_fragmetShader->Init();
    }

    m_initiated = true;
  }

  void Material::UnInit()
  {
    m_initiated = false;
  }

  Material* Material::GetCopy()
  {
    return new Material(*this);
  }

  RenderState* Material::GetRenderState()
  {
    if (m_diffuseTexture)
    {
      m_renderState.diffuseTexture = m_diffuseTexture->m_textureId;
    }

    if (m_cubeMap)
    {
      m_renderState.cubeMap = m_cubeMap->m_textureId;
    }

    return &m_renderState;
  }

  void Material::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
  }

  void Material::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    XmlNode* rootNode = parent;
    for (XmlNode* node = rootNode->first_node(); node; node = node->next_sibling())
    {
      if (String("diffuseTexture").compare(node->name()) == 0)
      {
        XmlAttribute* attr = node->first_attribute("name");
        m_diffuseTexture = GetTextureManager()->Create<Texture>(TexturePath(attr->value()));
      }
      else if (String("cubeMap").compare(node->name()) == 0)
      {
        XmlAttribute* attr = node->first_attribute("name");
        m_cubeMap = GetTextureManager()->Create<CubeMap>(TexturePath(attr->value()), ResourceType::CubeMap);
      }
      else if (String("shader").compare(node->name()) == 0)
      {
        XmlAttribute* attr = node->first_attribute("name");
        ShaderPtr shader = GetShaderManager()->Create<Shader>(ShaderPath(attr->value()));
        if (shader->m_shaderType == GL_VERTEX_SHADER)
        {
          m_vertexShader = shader;
        }
        else if (shader->m_shaderType == GL_FRAGMENT_SHADER)
        {
          m_fragmetShader = shader;
        }
        else
        {
          assert(false);
        }
      }
      else if (String("color").compare(node->name()) == 0)
      {
        ReadVec(node, m_color);
      }
      else
      {
        assert(false);
      }
    }
  }

  MaterialManager::MaterialManager()
  {
    m_type = ResourceType::Material;
  }

  MaterialManager::~MaterialManager()
  {
  }

  void MaterialManager::Init()
  {
    ResourceManager::Init();

    Material* material = new Material();
    material->m_vertexShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultVertex.shader"));
    material->m_fragmetShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultFragment.shader"));
    material->m_diffuseTexture = GetTextureManager()->Create<Texture>(TexturePath("default.png"));
    material->Init();

    m_storage[MaterialPath("default.material")] = MaterialPtr(material);

    material = new Material();
    material->m_vertexShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultVertex.shader"));
    material->m_fragmetShader = GetShaderManager()->Create<Shader>(ShaderPath("unlitFrag.shader"));
    material->m_diffuseTexture = GetTextureManager()->Create<Texture>(TexturePath("default.png"));
    material->Init();

    m_storage[MaterialPath("unlit.material")] = MaterialPtr(material);

    material = new Material();
    material->m_vertexShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultVertex.shader"));
    material->m_fragmetShader = GetShaderManager()->Create<Shader>(ShaderPath("solidColorFrag.shader"));
    material->Init();

    m_storage[MaterialPath("solid.material")] = MaterialPtr(material);

    material = new Material();
    material->m_vertexShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultVertex.shader"));
    material->m_fragmetShader = GetShaderManager()->Create<Shader>(ShaderPath("unlitColorFrag.shader"));
    material->Init();

    m_storage[MaterialPath("unlitSolid.material")] = MaterialPtr(material);
  }

  MaterialPtr MaterialManager::GetCopyOfUnlitMaterial()
  {
    return MaterialPtr(static_cast<Material*> (m_storage[MaterialPath("unlit.material")]->GetCopy()));
  }

  MaterialPtr MaterialManager::GetCopyOfUnlitColorMaterial()
  {
    return MaterialPtr(static_cast<Material*> (m_storage[MaterialPath("unlitSolid.material")]->GetCopy()));
  }

  MaterialPtr MaterialManager::GetCopyOfSolidMaterial()
  {
    return MaterialPtr(static_cast<Material*> (m_storage[MaterialPath("solid.material")]->GetCopy()));
  }

  MaterialPtr MaterialManager::GetCopyOfDefaultMaterial()
  {
    return MaterialPtr(static_cast<Material*> (m_storage[MaterialPath("default.material")]->GetCopy()));
  }

}
