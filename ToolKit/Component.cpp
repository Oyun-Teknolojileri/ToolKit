#include "Component.h"
#include "Mesh.h"
#include "Material.h"

namespace ToolKit
{

  ULongID Component::m_handle = NULL_HANDLE;

  Component::Component()
  {
    m_id = ++m_handle;
  }

  Component::~Component()
  {
  }

  ComponentType Component::GetType() const
  {
    return ComponentType::Base;
  }

  void Component::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* componentNode = CreateXmlNode(doc, "Component", parent);
    WriteAttr
    (
      componentNode, doc, "t", std::to_string
      (
        static_cast<int> (GetType())
      )
    );

    m_localData.Serialize(doc, componentNode);
  }

  void Component::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
  }

  MeshComponent::MeshComponent()
  {
    Mesh_Define
    (
      std::make_shared<ToolKit::Mesh>(),
      MeshComponentCategory.Name,
      MeshComponentCategory.Priority,
      true,
      true
    );

    Material_Define
    (
      std::make_shared<ToolKit::Material>(),
      MeshComponentCategory.Name,
      MeshComponentCategory.Priority,
      true,
      true
    );
  }

  MeshComponent::~MeshComponent()
  {
  }

  void MeshComponent::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
  }

  void MeshComponent::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
  }

  ComponentPtr MeshComponent::Copy()
  {
    MeshComponentPtr mc = std::make_shared<MeshComponent>();
    mc->m_localData = m_localData;
    if (Material())
    {
      // Deep copy declarative resources.
      Material() = Material()->Copy<ToolKit::Material>();
    }

    return mc;
  }

  BoundingBox MeshComponent::GetAABB()
  {
    return Mesh()->m_aabb;
  }

  void MeshComponent::Init(bool flushClientSideArray)
  {
    if (Mesh())
    {
      Mesh()->Init(flushClientSideArray);
    }

    if (Material())
    {
      Material()->Init(flushClientSideArray);
    }
  }

}  // namespace ToolKit
