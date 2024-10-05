/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "MeshComponent.h"

#include "Entity.h"
#include "Mesh.h"
#include "SkeletonComponent.h"

namespace ToolKit
{

  TKDefineClass(MeshComponent, Component);

  MeshComponent::MeshComponent() { m_boundingBox = infinitesimalBox; }

  MeshComponent::~MeshComponent() {}

  ComponentPtr MeshComponent::Copy(EntityPtr ntt)
  {
    MeshComponentPtr mc = MakeNewPtr<MeshComponent>();
    mc->m_localData     = m_localData;
    mc->m_entity        = ntt;
    return mc;
  }

  const BoundingBox& MeshComponent::GetBoundingBox()
  {
    SkeletonComponent* skelComp = OwnerEntity()->GetComponentFast<SkeletonComponent>();
    MeshPtr mesh                = GetMeshVal();
    if (skelComp && mesh->IsSkinned())
    {
      SkinMesh* skinMesh = (SkinMesh*) mesh.get();
      if (skelComp->isDirty)
      {
        m_boundingBox     = skinMesh->CalculateAABB(skelComp->GetSkeletonResourceVal().get(), skelComp->m_map);
        skelComp->isDirty = false;
      }
      return m_boundingBox;
    }

    return GetMeshVal()->m_boundingBox;
  }

  void MeshComponent::Init(bool flushClientSideArray) { GetMeshVal()->Init(flushClientSideArray); }

  XmlNode* MeshComponent::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    if (!m_serializableComponent)
    {
      return root;
    }

    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  void MeshComponent::ParameterConstructor()
  {
    Super::ParameterConstructor();

    Mesh_Define(MakeNewPtr<Mesh>(), MeshComponentCategory.Name, MeshComponentCategory.Priority, true, true);

    CastShadow_Define(true, MeshComponentCategory.Name, MeshComponentCategory.Priority, true, true);
  }

} // namespace ToolKit