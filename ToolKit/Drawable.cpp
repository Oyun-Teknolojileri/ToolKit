#include "Drawable.h"

#include <memory>

#include "Mesh.h"
#include "Material.h"
#include "ToolKit.h"
#include "Node.h"
#include "Util.h"
#include "Component.h"
#include "Skeleton.h"
#include "DebugNew.h"

namespace ToolKit
{

  Drawable::Drawable()
  {
    AddComponent(new MeshComponent());
  }

  Drawable::~Drawable()
  {
  }

  bool Drawable::IsDrawable() const
  {
    return true;
  }

  EntityType Drawable::GetType() const
  {
    return EntityType::Entity_Drawable;
  }

  void Drawable::SetPose(const AnimationPtr& anim, float time)
  {
    MeshPtr mesh = GetMesh();
    if (mesh->IsSkinned())
    {
      SkinMesh* skinMesh = static_cast<SkinMesh*> (mesh.get());
      SkeletonPtr skeleton = skinMesh->m_skeleton;
      anim->GetPose(skeleton, time);
    }
    else
    {
      anim->GetPose(m_node, time);
    }
  }

  BoundingBox Drawable::GetAABB(bool inWorld) const
  {
    BoundingBox bb = GetMesh()->m_aabb;
    if (inWorld)
    {
      TransformAABB(bb, m_node->GetTransform(TransformationSpace::TS_WORLD));
    }

    return bb;
  }

  Entity* Drawable::CopyTo(Entity* copyTo) const
  {
    return Entity::CopyTo(copyTo);
  }

  Entity* Drawable::InstantiateTo(Entity* copyTo) const
  {
    return Entity::InstantiateTo(copyTo);
  }

  void Drawable::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Entity::Serialize(doc, parent);
  }

  void Drawable::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    ClearComponents();
    Entity::DeSerialize(doc, parent);
  }

  void Drawable::RemoveResources()
  {
    GetMeshManager()->Remove(GetMesh()->GetFile());
  }

  MeshPtr& Drawable::GetMesh() const
  {
    MeshComponentPtr meshComp = GetComponent<MeshComponent>();
    return meshComp->Mesh();
  }

  void Drawable::SetMesh(const MeshPtr& mesh)
  {
    MeshComponentPtr meshComp = GetComponent<MeshComponent>();
    meshComp->Mesh() = mesh;
  }

}  // namespace ToolKit

