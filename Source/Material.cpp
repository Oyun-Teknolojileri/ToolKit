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

  void Material::Save(bool onlyIfDirty)
  {
    Resource::Save(onlyIfDirty);
    m_vertexShader->Save(onlyIfDirty);
    m_fragmetShader->Save(onlyIfDirty);
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
      m_vertexShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultVertex.shader", true));
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
        m_fragmetShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultFragment.shader", true));
      }
      else
      {
        m_fragmetShader = GetShaderManager()->Create<Shader>(ShaderPath("solidColorFrag.shader", true));
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
    Material* cpyMat = new Material(*this);
    cpyMat->m_dirty = true;

    if (!m_file.empty())
    {
      String filePath = CreateCopyFileFullPath(m_file);
      cpyMat->m_file = GetRelativeResourcePath(filePath);
    }

    return cpyMat;
  }

  RenderState* Material::GetRenderState()
  {
    if (m_diffuseTexture)
    {
      m_renderState.diffuseTextureInUse = true;
      m_renderState.diffuseTexture = m_diffuseTexture->m_textureId;
    }
    else
    {
      m_renderState.diffuseTextureInUse = false;
    }

    if (m_cubeMap)
    {
      m_renderState.cubeMap = m_cubeMap->m_textureId;
    }
    else
    {
      m_renderState.cubeMap = false;
    }

    return &m_renderState;
  }

  void Material::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* container = doc->allocate_node
    (
      rapidxml::node_type::node_element,
      "material"
    );

    if (parent != nullptr)
    {
      parent->append_node(container);
    }
    else
    {
      doc->append_node(container);
    }

    if (m_diffuseTexture)
    {
      XmlNode* node = doc->allocate_node
      (
        rapidxml::node_type::node_element,
        "diffuseTexture"
      );
      container->append_node(node);

      String file = m_diffuseTexture->m_file;
      WriteAttr(node, doc, "name", file);
    }

    if (m_cubeMap)
    {
      XmlNode* node = doc->allocate_node
      (
        rapidxml::node_type::node_element,
        "cubeMap"
      );
      container->append_node(node);

      String file = m_cubeMap->m_file;
      WriteAttr(node, doc, "name", file);
    }

    if (m_vertexShader)
    {
      XmlNode* node = doc->allocate_node
      (
        rapidxml::node_type::node_element,
        "shader"
      );
      container->append_node(node);

      WriteAttr(node, doc, "name", m_vertexShader->m_file);
    }

    if (m_fragmetShader)
    {
      XmlNode* node = doc->allocate_node
      (
        rapidxml::node_type::node_element,
        "shader"
      );
      container->append_node(node);

      WriteAttr(node, doc, "name", m_fragmetShader->m_file);
    }

    XmlNode* node = doc->allocate_node
    (
      rapidxml::node_type::node_element,
      "color"
    );
    container->append_node(node);
    WriteVec(node, doc, m_color);
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
        m_cubeMap = GetTextureManager()->Create<CubeMap>(TexturePath(attr->value()));
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
    material->m_vertexShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultVertex.shader", true));
    material->m_fragmetShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultFragment.shader", true));
    material->m_diffuseTexture = GetTextureManager()->Create<Texture>(TexturePath("default.png", true));
    material->Init();

    m_storage[MaterialPath("default.material", true)] = MaterialPtr(material);

    material = new Material();
    material->m_vertexShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultVertex.shader", true));
    material->m_fragmetShader = GetShaderManager()->Create<Shader>(ShaderPath("unlitFrag.shader", true));
    material->m_diffuseTexture = GetTextureManager()->Create<Texture>(TexturePath("default.png", true));
    material->Init();

    m_storage[MaterialPath("unlit.material", true)] = MaterialPtr(material);

    material = new Material();
    material->m_vertexShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultVertex.shader", true));
    material->m_fragmetShader = GetShaderManager()->Create<Shader>(ShaderPath("solidColorFrag.shader", true));
    material->Init();

    m_storage[MaterialPath("solid.material", true)] = MaterialPtr(material);

    material = new Material();
    material->m_vertexShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultVertex.shader", true));
    material->m_fragmetShader = GetShaderManager()->Create<Shader>(ShaderPath("unlitColorFrag.shader", true));
    material->Init();

    m_storage[MaterialPath("unlitSolid.material", true)] = MaterialPtr(material);
  }

  MaterialPtr MaterialManager::GetCopyOfUnlitMaterial()
  {
    return MaterialPtr(static_cast<Material*> (m_storage[MaterialPath("unlit.material", true)]->GetCopy()));
  }

  MaterialPtr MaterialManager::GetCopyOfUnlitColorMaterial()
  {
    return MaterialPtr(static_cast<Material*> (m_storage[MaterialPath("unlitSolid.material", true)]->GetCopy()));
  }

  MaterialPtr MaterialManager::GetCopyOfSolidMaterial()
  {
    return MaterialPtr(static_cast<Material*> (m_storage[MaterialPath("solid.material", true)]->GetCopy()));
  }

  MaterialPtr MaterialManager::GetCopyOfDefaultMaterial()
  {
    return MaterialPtr(static_cast<Material*> (m_storage[MaterialPath("default.material", true)]->GetCopy()));
  }

}
