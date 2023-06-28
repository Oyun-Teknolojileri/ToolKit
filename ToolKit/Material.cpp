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
    m_alpha = 1.0f;
  }

  Material::Material(const String& file) : Material() { SetFile(file); }

  Material::~Material() { UnInit(); }

  void Material::Load()
  {
    if (m_loaded)
    {
      return;
    }

    XmlFilePtr file = GetFileManager()->GetXmlFile(GetFile());
    XmlDocument doc;
    doc.parse<0>(file->data());

    XmlNode* rootNode = doc.first_node("material");
    DeSerialize(&doc, rootNode);

    m_loaded = true;
  }

  void Material::Save(bool onlyIfDirty)
  {
    Resource::Save(onlyIfDirty);
    if (m_vertexShader)
    {
      m_vertexShader->Save(onlyIfDirty);
    }

    if (m_fragmentShader)
    {
      m_fragmentShader->Save(onlyIfDirty);
    }
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
    }

    if (m_emissiveTexture)
    {
      m_emissiveTexture->Init(flushClientSideArray);
    }

    if (m_metallicRoughnessTexture)
    {
      if (m_metallicRoughnessTexture->GetTextureSettings().MinFilter != GraphicTypes::SampleNearest)
      {
        m_metallicRoughnessTexture->UnInit();
        m_metallicRoughnessTexture->Load();

        TextureSettings set;
        set.InternalFormat = GraphicTypes::FormatRGBA;
        set.MinFilter      = GraphicTypes::SampleNearest;
        set.Type           = GraphicTypes::TypeUnsignedByte;
        set.GenerateMipMap = false;
        m_metallicRoughnessTexture->SetTextureSettings(set);
      }

      m_metallicRoughnessTexture->Init(flushClientSideArray);
    }

    if (m_normalMap)
    {
      if (m_normalMap->GetTextureSettings().MinFilter != GraphicTypes::SampleNearest)
      {
        m_normalMap->UnInit();
        m_normalMap->Load();

        TextureSettings set;
        set.InternalFormat = GraphicTypes::FormatRGBA;
        set.MinFilter      = GraphicTypes::SampleNearest;
        set.Type           = GraphicTypes::TypeUnsignedByte;
        set.GenerateMipMap = false;
        m_normalMap->SetTextureSettings(set);
      }

      m_normalMap->Init(flushClientSideArray);
    }

    if (m_cubeMap)
    {
      m_cubeMap->Init(flushClientSideArray);
    }

    if (m_vertexShader)
    {
      m_vertexShader->Init();
    }
    else
    {
      m_vertexShader = GetShaderManager()->GetDefaultVertexShader();
      m_vertexShader->Init();
    }

    if (m_fragmentShader)
    {
      m_fragmentShader->Init();
    }
    else
    {
      m_fragmentShader = GetShaderManager()->GetPbrForwardShader();
      m_fragmentShader->Init();
    }

    m_initiated = true;
  }

  void Material::UnInit() { m_initiated = false; }

  void Material::CopyTo(Resource* other)
  {
    Resource::CopyTo(other);
    Material* cpy                   = static_cast<Material*>(other);
    cpy->m_cubeMap                  = m_cubeMap;
    cpy->m_diffuseTexture           = m_diffuseTexture;
    cpy->m_vertexShader             = m_vertexShader;
    cpy->m_fragmentShader           = m_fragmentShader;
    cpy->m_color                    = m_color;
    cpy->m_alpha                    = m_alpha;
    cpy->m_metallic                 = m_metallic;
    cpy->m_roughness                = m_roughness;
    cpy->m_renderState              = m_renderState;
    cpy->m_dirty                    = true;
    cpy->m_emissiveTexture          = m_emissiveTexture;
    cpy->m_emissiveColor            = m_emissiveColor;
    cpy->m_metallicRoughnessTexture = m_metallicRoughnessTexture;
    cpy->m_normalMap                = m_normalMap;
    cpy->m_normalMap                = m_normalMap;
  }

  RenderState* Material::GetRenderState() { return &m_renderState; }

  void Material::SetRenderState(RenderState* state)
  {
    m_renderState = *state; // Copy
  }

  void Material::SetDefaultMaterialTypeShaders()
  {
    if (IsPBR())
    {
      if (IsTranslucent())
      {
        m_fragmentShader = GetShaderManager()->GetPbrForwardShader();
      }
      else
      {
        m_fragmentShader = GetShaderManager()->GetPbrDefferedShader();
      }

      m_fragmentShader->Init();
    }
  }

  void Material::SetAlpha(float val)
  {
    val                         = glm::clamp(val, 0.0f, 1.0f);
    m_alpha                     = val;
    bool isForward              = m_alpha < 0.99f;
    m_renderState.blendFunction = isForward ? BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA : BlendFunction::NONE;
  }

  float& Material::GetAlpha() { return m_alpha; }

  bool Material::IsDeferred()
  {
    if (IsTranslucent())
    {
      return false;
    }

    return m_fragmentShader->GetFile() == GetShaderManager()->PbrDefferedShaderFile();
  }

  bool Material::IsTranslucent() { return m_renderState.blendFunction == BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA; }

  bool Material::IsPBR()
  {
    const String& file = m_fragmentShader->GetFile();
    return file == GetShaderManager()->PbrDefferedShaderFile() || file == GetShaderManager()->PbrForwardShaderFile();
  }

  XmlNode* Material::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* container = CreateXmlNode(doc, "material", parent);

    if (m_diffuseTexture)
    {
      XmlNode* node = CreateXmlNode(doc, "diffuseTexture", container);
      String file   = GetRelativeResourcePath(m_diffuseTexture->GetSerializeFile());
      WriteAttr(node, doc, XmlNodeName.data(), file);
    }

    if (m_cubeMap)
    {
      XmlNode* node = CreateXmlNode(doc, "cubeMap", container);
      String file   = GetRelativeResourcePath(m_cubeMap->GetSerializeFile());
      WriteAttr(node, doc, XmlNodeName.data(), file);
    }

    if (m_vertexShader)
    {
      XmlNode* node = CreateXmlNode(doc, "shader", container);
      String file   = GetRelativeResourcePath(m_vertexShader->GetSerializeFile());
      WriteAttr(node, doc, XmlNodeName.data(), file);
    }

    if (m_fragmentShader)
    {
      XmlNode* node = CreateXmlNode(doc, "shader", container);
      String file   = GetRelativeResourcePath(m_fragmentShader->GetSerializeFile());
      WriteAttr(node, doc, XmlNodeName.data(), file);
    }

    if (m_emissiveTexture)
    {
      XmlNode* node = CreateXmlNode(doc, "emissiveTexture", container);
      String file   = GetRelativeResourcePath(m_emissiveTexture->GetSerializeFile());
      WriteAttr(node, doc, XmlNodeName.data(), file);
    }

    if (m_metallicRoughnessTexture)
    {
      XmlNode* node = CreateXmlNode(doc, "metallicRoughnessTexture", container);
      String file   = GetRelativeResourcePath(m_metallicRoughnessTexture->GetSerializeFile());
      WriteAttr(node, doc, XmlNodeName.data(), file);
    }

    if (m_normalMap)
    {
      XmlNode* node = CreateXmlNode(doc, "normalMap", container);
      String file   = GetRelativeResourcePath(m_normalMap->GetSerializeFile());
      WriteAttr(node, doc, XmlNodeName.data(), file);
    }

    XmlNode* node = CreateXmlNode(doc, "color", container);
    WriteVec(node, doc, m_color);

    node = CreateXmlNode(doc, "emissiveColor", container);
    WriteVec(node, doc, m_emissiveColor);

    node = CreateXmlNode(doc, "alpha", container);
    WriteAttr(node, doc, XmlNodeName.data(), std::to_string(m_alpha));

    node = CreateXmlNode(doc, "metallic", container);
    WriteAttr(node, doc, XmlNodeName.data(), std::to_string(m_metallic));

    node = CreateXmlNode(doc, "roughness", container);
    WriteAttr(node, doc, XmlNodeName.data(), std::to_string(m_roughness));

    node = CreateXmlNode(doc, "materialType", container);
    WriteAttr(node, doc, XmlNodeName.data(), std::to_string((int) m_materialType));

    m_renderState.Serialize(doc, container);
    return container;
  }

  void Material::DeSerializeImp(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    XmlNode* rootNode = parent;
    for (XmlNode* node = rootNode->first_node(); node; node = node->next_sibling())
    {
      if (strcmp("diffuseTexture", node->name()) == 0)
      {
        XmlAttribute* attr = node->first_attribute(XmlNodeName.data());
        String path        = attr->value();
        NormalizePath(path);
        m_diffuseTexture = GetTextureManager()->Create<Texture>(TexturePath(path));
      }
      else if (strcmp("cubeMap", node->name()) == 0)
      {
        XmlAttribute* attr = node->first_attribute(XmlNodeName.data());
        String path        = attr->value();
        NormalizePath(path);
        m_cubeMap = GetTextureManager()->Create<CubeMap>(TexturePath(path));
      }
      else if (strcmp("shader", node->name()) == 0)
      {
        XmlAttribute* attr = node->first_attribute(XmlNodeName.data());
        String path        = attr->value();
        NormalizePath(path);
        ShaderPtr shader = GetShaderManager()->Create<Shader>(ShaderPath(path));
        if (shader->m_shaderType == ShaderType::VertexShader)
        {
          m_vertexShader = shader;
        }
        else if (shader->m_shaderType == ShaderType::FragmentShader)
        {
          m_fragmentShader = shader;
        }
        else
        {
          assert(false);
        }
      }
      else if (strcmp("color", node->name()) == 0)
      {
        ReadVec(node, m_color);
      }
      else if (strcmp("alpha", node->name()) == 0)
      {
        ReadAttr(node, XmlNodeName.data(), m_alpha);
      }
      else if (strcmp("renderState", node->name()) == 0)
      {
        m_renderState.DeSerialize(doc, parent);
      }
      else if (strcmp("emissiveTexture", node->name()) == 0)
      {
        XmlAttribute* attr = node->first_attribute(XmlNodeName.data());
        String path        = attr->value();
        NormalizePath(path);
        m_emissiveTexture = GetTextureManager()->Create<Texture>(TexturePath(path));
      }
      else if (strcmp("emissiveColor", node->name()) == 0)
      {
        ReadVec(node, m_emissiveColor);
      }
      else if (strcmp("metallicRoughnessTexture", node->name()) == 0)
      {
        XmlAttribute* attr = node->first_attribute(XmlNodeName.data());
        String path        = attr->value();
        NormalizePath(path);
        m_metallicRoughnessTexture = GetTextureManager()->Create<Texture>(TexturePath(path));
      }
      else if (strcmp("normalMap", node->name()) == 0)
      {
        XmlAttribute* attr = node->first_attribute(XmlNodeName.data());
        String path        = attr->value();
        NormalizePath(path);
        m_normalMap = GetTextureManager()->Create<Texture>(TexturePath(path));
      }
      else if (strcmp("metallic", node->name()) == 0)
      {
        ReadAttr(node, XmlNodeName.data(), m_metallic);
      }
      else if (strcmp("roughness", node->name()) == 0)
      {
        ReadAttr(node, XmlNodeName.data(), m_roughness);
      }
      else if (strcmp("materialType", node->name()) == 0)
      {
        int matType;
        ReadAttr(node, XmlNodeName.data(), matType);
        matType        = glm::clamp(matType, 1, 2);
        m_materialType = (MaterialType) matType;
      }
      else
      {
        assert(false);
      }
    }

    // Update old materials than v0.4.0

    // If no shader provided, assign a proper default.
    if (m_fragmentShader == nullptr)
    {
      if (IsTranslucent())
      {
        m_fragmentShader = GetShaderManager()->GetPbrForwardShader();
      }
      else
      {
        m_fragmentShader = GetShaderManager()->GetPbrDefferedShader();
      }
    }
    else
    {
      // Can this be draw in deferred path ?
      if (!IsDeferred())
      {
        // If not using a custom shader.
        if (m_fragmentShader->GetFile() == GetShaderManager()->PbrForwardShaderFile())
        {
          // And not translucent.
          if (!IsTranslucent())
          {
            // Draw in deferred.
            m_fragmentShader = GetShaderManager()->GetPbrDefferedShader();
          }
        }
      }
    }
  }

  MaterialManager::MaterialManager() { m_type = ResourceType::Material; }

  MaterialManager::~MaterialManager() {}

  void MaterialManager::Init()
  {
    ResourceManager::Init();

    Material* material         = new Material();
    material->m_vertexShader   = GetShaderManager()->Create<Shader>(ShaderPath("defaultVertex.shader", true));
    material->m_fragmentShader = GetShaderManager()->GetPbrDefferedShader();
    material->m_diffuseTexture = GetTextureManager()->Create<Texture>(TexturePath("default.png", true));
    material->Init();

    m_storage[MaterialPath("default.material", true)] = MaterialPtr(material);

    material                                          = new Material();
    material->m_materialType                          = MaterialType::Custom;

    material->m_vertexShader   = GetShaderManager()->Create<Shader>(ShaderPath("defaultVertex.shader", true));

    material->m_fragmentShader = GetShaderManager()->Create<Shader>(ShaderPath("unlitFrag.shader", true));

    material->m_diffuseTexture = GetTextureManager()->Create<Texture>(TexturePath("default.png", true));

    material->Init();

    m_storage[MaterialPath("unlit.material", true)] = MaterialPtr(material);
  }

  bool MaterialManager::CanStore(ResourceType t) { return t == ResourceType::Material; }

  ResourcePtr MaterialManager::CreateLocal(ResourceType type) { return ResourcePtr(new Material()); }

  String MaterialManager::GetDefaultResource(ResourceType type) { return MaterialPath("missing.material", true); }

  MaterialPtr MaterialManager::GetCopyOfUnlitMaterial()
  {
    return m_storage[MaterialPath("unlit.material", true)]->Copy<Material>();
  }

  MaterialPtr MaterialManager::GetCopyOfUIMaterial()
  {
    MaterialPtr material                      = GetMaterialManager()->GetCopyOfUnlitMaterial();
    material->GetRenderState()->blendFunction = BlendFunction::ALPHA_MASK;

    return material;
  }

  MaterialPtr MaterialManager::GetCopyOfUnlitColorMaterial()
  {
    MaterialPtr umat       = GetCopyOfUnlitMaterial();
    umat->m_diffuseTexture = nullptr;
    return umat;
  }

  MaterialPtr MaterialManager::GetCopyOfDefaultMaterial()
  {
    return m_storage[MaterialPath("default.material", true)]->Copy<Material>();
  }

} // namespace ToolKit
