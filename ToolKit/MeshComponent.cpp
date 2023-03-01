#include "MeshComponent.h"
#include "Mesh.h"
#include "SkeletonComponent.h"
#include "Entity.h"

#include "DebugNew.h"

namespace ToolKit
{

  MeshComponent::MeshComponent()
  {
    Mesh_Define(std::make_shared<ToolKit::Mesh>(),
                MeshComponentCategory.Name,
                MeshComponentCategory.Priority,
                true,
                true);

    CastShadow_Define(true,
                      MeshComponentCategory.Name,
                      MeshComponentCategory.Priority,
                      true,
                      true);
  }

  MeshComponent::~MeshComponent() {}

  ComponentPtr MeshComponent::Copy(Entity* ntt)
  {
    MeshComponentPtr mc = std::make_shared<MeshComponent>();
    mc->m_localData     = m_localData;
    mc->m_entity        = ntt;
    return mc;
  }

  BoundingBox MeshComponent::GetAABB()
  {
    SkeletonComponentPtr skelComp = m_entity->GetComponent<SkeletonComponent>();
    if (skelComp && GetMeshVal()->IsSkinned())
    {
      SkinMesh* skinMesh = (SkinMesh*) GetMeshVal().get();
      if (skelComp->isDirty)
      {
        m_aabb =
            skinMesh->CalculateAABB(skelComp->GetSkeletonResourceVal().get(),
                                    skelComp->m_map);
        skelComp->isDirty = false;
      }
      return m_aabb;
    }
    return GetMeshVal()->m_aabb;
  }

  void MeshComponent::Init(bool flushClientSideArray)
  {
    GetMeshVal()->Init(flushClientSideArray);
  }

} // namespace ToolKit