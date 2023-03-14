#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "Material.h"
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

  void MaterialComponent::Init(bool flushClientSideArray)
  {
    GetMaterialVal()->Init(flushClientSideArray);
  }

  ComponentPtr MaterialComponent::Copy(Entity* ntt)
  {
    MaterialComponentPtr mc = std::make_shared<MaterialComponent>();
    mc->m_localData         = m_localData;
    mc->m_entity            = ntt;

    return mc;
  }

  MultiMaterialComponent::MultiMaterialComponent() {}

  MultiMaterialComponent::~MultiMaterialComponent() {}

  ComponentPtr MultiMaterialComponent::Copy(Entity* ntt)
  {
    MultiMaterialPtr mc = std::make_shared<MultiMaterialComponent>();
    mc->m_localData     = m_localData;
    mc->m_entity        = ntt;
    mc->materials       = materials;

    return mc;
  }

  void MultiMaterialComponent::Init(bool flushClientSideArray) {}

  const char *XmlMatCountAttrib = "MaterialCount", *XmlMatIdAttrib = "ID";

  void MultiMaterialComponent::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Component::DeSerialize(doc, parent);
    uint matCount = 0;
    ReadAttr(parent, XmlMatCountAttrib, matCount);
    materials.resize(matCount);
    for (uint i = 0; i < materials.size(); i++)
    {
      XmlNode* resourceNode = parent->first_node(std::to_string(i).c_str());
      if (!resourceNode)
      {
        materials[i] = GetMaterialManager()->GetCopyOfDefaultMaterial();
        continue;
      }
      materials[i] = GetMaterialManager()->Create<Material>(
          MaterialPath(Resource::DeserializeRef(resourceNode)));
    }
  }

  void MultiMaterialComponent::Serialize(XmlDocument* doc,
                                         XmlNode* parent) const
  {
    Component::Serialize(doc, parent);
    XmlNode* compNode = parent->last_node(XmlComponent.c_str());
    WriteAttr(compNode,
              doc,
              XmlMatCountAttrib,
              std::to_string(materials.size()));
    for (uint i = 0; i < materials.size(); i++)
    {
      if (materials[i]->m_dirty)
      {
        materials[i]->Save(true);
      }
      XmlNode* resourceRefNode =
          CreateXmlNode(doc, std::to_string(i), compNode);
      materials[i]->SerializeRef(doc, resourceRefNode);
    }
  }

  void MultiMaterialComponent::AddMaterial(MaterialPtr mat)
  {
    materials.push_back(mat);
  }

  void MultiMaterialComponent::RemoveMaterial(uint index)
  {
    assert(materials.size() >= index && "Material List overflow");
    materials.erase(materials.begin() + index);
  }

  const MaterialPtrArray& MultiMaterialComponent::GetMaterialList() const
  {
    return materials;
  }

  MaterialPtrArray& MultiMaterialComponent::GetMaterialList()
  {
    return materials;
  }

  void MultiMaterialComponent::UpdateMaterialList()
  {
    materials.clear();
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
        materials.push_back(meshCollector[i]->m_material);
      }
    }
  }

} // namespace ToolKit