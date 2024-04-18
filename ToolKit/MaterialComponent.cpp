/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "MaterialComponent.h"

#include "Entity.h"
#include "Material.h"
#include "Mesh.h"
#include "MeshComponent.h"
#include "ToolKit.h"



namespace ToolKit
{

  TKDefineClass(MaterialComponent, Component);

  MaterialComponent::MaterialComponent() {}

  MaterialComponent::~MaterialComponent() {}

  ComponentPtr MaterialComponent::Copy(EntityPtr ntt)
  {
    MaterialComponentPtr mc = MakeNewPtr<MaterialComponent>();
    mc->m_localData         = m_localData;
    mc->m_entity            = ntt;
    mc->m_materialList      = m_materialList;

    return mc;
  }

  void MaterialComponent::Init(bool flushClientSideArray) {}

  const char *XmlMatCountAttrib = "MaterialCount", *XmlMatIdAttrib = "ID";

  void MaterialComponent::ParameterConstructor() { Super::ParameterConstructor(); }

  XmlNode* MaterialComponent::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* compNode = Super::SerializeImp(doc, parent);
    if (!m_serializableComponent)
    {
      return compNode;
    }

    XmlNode* matNode = CreateXmlNode(doc, StaticClass()->Name, compNode);

    WriteAttr(matNode, doc, XmlMatCountAttrib, std::to_string(m_materialList.size()));
    for (size_t i = 0; i < m_materialList.size(); i++)
    {
      if (m_materialList[i]->m_dirty)
      {
        m_materialList[i]->Save(true);
      }
      XmlNode* resourceRefNode = CreateXmlNode(doc, std::to_string(i), matNode);
      m_materialList[i]->SerializeRef(doc, resourceRefNode);
    }

    return matNode;
  }

  XmlNode* MaterialComponent::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    if (m_version >= TKV045)
    {
      return DeSerializeImpV045(info, parent);
    }

    // Old file, keep parsing.
    XmlNode* compNode = Super::DeSerializeImp(info, parent);
    uint matCount     = 0;

    ReadAttr(parent, XmlMatCountAttrib, matCount);
    m_materialList.resize(matCount);
    for (size_t i = 0; i < m_materialList.size(); i++)
    {
      XmlNode* resourceNode = parent->first_node(std::to_string(i).c_str());
      if (resourceNode == nullptr)
      {
        m_materialList[i] = GetMaterialManager()->GetCopyOfDefaultMaterial();
      }
      else
      {
        String matRef     = Resource::DeserializeRef(resourceNode);
        String path       = MaterialPath(matRef);
        m_materialList[i] = GetMaterialManager()->Create<Material>(path);
      }
    }

    return compNode->first_node(StaticClass()->Name.c_str());
  }

  XmlNode* MaterialComponent::DeSerializeImpV045(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlNode* comNode = Super::DeSerializeImp(info, parent);
    XmlNode* matNode = comNode->first_node(StaticClass()->Name.c_str());

    uint matCount    = 0;
    ReadAttr(matNode, XmlMatCountAttrib, matCount);
    m_materialList.resize(matCount);

    for (size_t i = 0; i < m_materialList.size(); i++)
    {
      XmlNode* resourceNode = matNode->first_node(std::to_string(i).c_str());
      if (resourceNode)
      {
        String matRef     = Resource::DeserializeRef(resourceNode);
        m_materialList[i] = GetMaterialManager()->Create<Material>(MaterialPath(matRef));
      }
      else
      {
        m_materialList[i] = GetMaterialManager()->GetCopyOfDefaultMaterial();
        continue;
      }
    }

    return matNode;
  }

  void MaterialComponent::AddMaterial(MaterialPtr mat) { m_materialList.push_back(mat); }

  void MaterialComponent::RemoveMaterial(uint index)
  {
    assert(m_materialList.size() >= index && "Material List overflow");
    m_materialList.erase(m_materialList.begin() + index);
  }

  const MaterialPtrArray& MaterialComponent::GetMaterialList() const { return m_materialList; }

  MaterialPtrArray& MaterialComponent::GetMaterialList() { return m_materialList; }

  void MaterialComponent::UpdateMaterialList()
  {
    m_materialList.clear();

    MeshComponentPtr meshComp;
    if (EntityPtr owner = OwnerEntity())
    {
      meshComp = owner->GetComponent<MeshComponent>();
      if (meshComp != nullptr && meshComp->GetMeshVal() != nullptr)
      {
        MeshRawPtrArray meshCollector;
        meshComp->GetMeshVal()->GetAllMeshes(meshCollector, true);

        for (uint i = 0; i < meshCollector.size(); i++)
        {
          m_materialList.push_back(meshCollector[i]->m_material);
        }
      }
    }
  }

  MaterialPtr MaterialComponent::GetFirstMaterial()
  {
    if (m_materialList.empty())
    {
      if (const MeshComponentPtr& mc = OwnerEntity()->GetMeshComponent())
      {
        if (MeshPtr m = mc->GetMeshVal())
        {
          if (m->m_material)
          {
            // Either return the material on the mesh.
            return m->m_material;
          }
        }
      }
    }
    else
    {
      // First found material.
      return m_materialList.front();
    }

    // Worst case, a default material.
    return GetMaterialManager()->GetCopyOfDefaultMaterial(false);
  }

  void MaterialComponent::SetFirstMaterial(const MaterialPtr& material)
  {
    if (m_materialList.empty())
    {
      m_materialList.push_back(material);
    }
    else
    {
      m_materialList[0] = material;
    }
  }

} // namespace ToolKit