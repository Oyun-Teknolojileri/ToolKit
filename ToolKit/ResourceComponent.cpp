#include "ResourceComponent.h"
#include "Mesh.h"
#include "Material.h"
#include "Entity.h"
#include "ToolKit.h"

namespace ToolKit
{

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
  }

  MeshComponent::~MeshComponent()
  {
  }

  ComponentPtr MeshComponent::Copy(Entity* ntt)
  {
    MeshComponentPtr mc = std::make_shared<MeshComponent>();
    mc->m_localData = m_localData;
    mc->m_entity = ntt;

    return mc;
  }

  BoundingBox MeshComponent::GetAABB()
  {
    return GetMeshVal()->m_aabb;
  }

  void MeshComponent::Init(bool flushClientSideArray)
  {
    GetMeshVal()->Init(flushClientSideArray);
  }

  MaterialComponent::MaterialComponent()
  {
    Material_Define
    (
      nullptr,
      MaterialComponentCategory.Name,
      MaterialComponentCategory.Priority,
      true,
      true
    );
  }

  MaterialComponent::~MaterialComponent()
  {
  }

  void MaterialComponent::Init(bool flushClientSideArray)
  {
    GetMaterialVal()->Init(flushClientSideArray);
  }

  ComponentPtr MaterialComponent::Copy(Entity* ntt)
  {
    MaterialComponentPtr mc = std::make_shared<MaterialComponent>();
    mc->m_localData = m_localData;
    mc->m_entity = ntt;

    return mc;
  }

}  //  namespace ToolKit
