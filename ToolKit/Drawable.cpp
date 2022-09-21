#include "Drawable.h"

#include "Component.h"
#include "Material.h"
#include "Mesh.h"
#include "Node.h"
#include "ResourceComponent.h"
#include "Skeleton.h"
#include "ToolKit.h"
#include "Util.h"

#include <memory>

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

  EntityType Drawable::GetType() const
  {
    return EntityType::Entity_Drawable;
  }

  void Drawable::SetPose(const AnimationPtr& anim, float time)
  {
    Entity::SetPose(anim, time);
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
    Entity::DeSerialize(doc, parent);
  }

  void Drawable::RemoveResources()
  {
    GetMeshManager()->Remove(GetMesh()->GetFile());
  }

  MeshPtr Drawable::GetMesh() const
  {
    MeshComponentPtr meshComp = GetComponent<MeshComponent>();
    return meshComp->GetMeshVal();
  }

  void Drawable::SetMesh(const MeshPtr& mesh)
  {
    MeshComponentPtr meshComp = GetComponent<MeshComponent>();
    meshComp->SetMeshVal(mesh);
  }

} // namespace ToolKit
