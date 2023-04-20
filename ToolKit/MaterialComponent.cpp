#include "MaterialComponent.h"

#include "Material.h"
#include "MeshComponent.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  MaterialComponent::MaterialComponent()
  {
    Material_Define(GetMaterialManager()->GetCopyOfDefaultMaterial(),
                    MaterialComponentCategory.Name,
                    MaterialComponentCategory.Priority,
                    true,
                    true);
  }

  MaterialComponent::~MaterialComponent() {}

  ComponentPtr MaterialComponent::Copy(Entity* ntt)
  {
    MaterialComponentPtr mc = std::make_shared<MaterialComponent>();
    mc->m_localData         = m_localData;
    mc->m_entity            = ntt;
    mc->m_materialList      = m_materialList;

    return mc;
  }

  void MaterialComponent::Init(bool flushClientSideArray) {}

  const char *XmlMatCountAttrib = "MaterialCount", *XmlMatIdAttrib = "ID";

  void MaterialComponent::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Component::DeSerialize(doc, parent);
    uint matCount = 0;
    ReadAttr(parent, XmlMatCountAttrib, matCount);
    m_materialList.resize(matCount);
    for (size_t i = 0; i < m_materialList.size(); i++)
    {
      XmlNode* resourceNode = parent->first_node(std::to_string(i).c_str());
      if (!resourceNode)
      {
        m_materialList[i] = GetMaterialManager()->GetCopyOfDefaultMaterial();
        continue;
      }
      m_materialList[i] = GetMaterialManager()->Create<Material>(
          MaterialPath(Resource::DeserializeRef(resourceNode)));
    }

    // Deprecated Here for compatibility with 0.4.1
    if (m_materialList.empty()) 
    {
      if (MaterialPtr mat = GetMaterialVal()) 
      {
        // Transfer the old material in to the list.
        m_materialList.push_back(mat);
        m_localData.Remove(ParamMaterial().m_id);
      }
    }

  }

  void MaterialComponent::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Component::Serialize(doc, parent);
    XmlNode* compNode = parent->last_node(XmlComponent.c_str());
    WriteAttr(compNode,
              doc,
              XmlMatCountAttrib,
              std::to_string(m_materialList.size()));
    for (size_t i = 0; i < m_materialList.size(); i++)
    {
      if (m_materialList[i]->m_dirty)
      {
        m_materialList[i]->Save(true);
      }
      XmlNode* resourceRefNode =
          CreateXmlNode(doc, std::to_string(i), compNode);
      m_materialList[i]->SerializeRef(doc, resourceRefNode);
    }
  }

  void MaterialComponent::AddMaterial(MaterialPtr mat)
  {
    m_materialList.push_back(mat);
  }

  void MaterialComponent::RemoveMaterial(uint index)
  {
    assert(m_materialList.size() >= index && "Material List overflow");
    m_materialList.erase(m_materialList.begin() + index);
  }

  const MaterialPtrArray& MaterialComponent::GetMaterialList() const
  {
    return m_materialList;
  }

  MaterialPtrArray& MaterialComponent::GetMaterialList()
  {
    return m_materialList;
  }

  void MaterialComponent::UpdateMaterialList()
  {
    m_materialList.clear();
    MeshComponentPtrArray meshComps;
    m_entity->GetComponent<MeshComponent>(meshComps);

    for (MeshComponentPtr meshComp : meshComps)
    {
      if (meshComp == nullptr || meshComp->GetMeshVal() == nullptr)
      {
        continue;
      }
      MeshRawPtrArray meshCollector;
      meshComp->GetMeshVal()->GetAllMeshes(meshCollector);

      for (uint i = 0; i < meshCollector.size(); i++)
      {
        m_materialList.push_back(meshCollector[i]->m_material);
      }
    }
  }

  MaterialPtr MaterialComponent::GetFirstMaterial()
  {
    if (m_materialList.empty())
    {
      if (const MeshComponentPtr& mc = m_entity->GetMeshComponent())
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
    return GetMaterialManager()->GetCopyOfDefaultMaterial();
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