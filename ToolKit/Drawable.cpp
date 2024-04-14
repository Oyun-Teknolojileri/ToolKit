/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Drawable.h"

#include "AABBOverrideComponent.h"
#include "Component.h"
#include "Material.h"
#include "Mesh.h"
#include "Node.h"
#include "Skeleton.h"
#include "ToolKit.h"
#include "Util.h"

#include <memory>



namespace ToolKit
{

  TKDefineClass(Drawable, Entity);

  Drawable::Drawable() { AddComponent<MeshComponent>(); }

  Drawable::~Drawable() {}

  void Drawable::SetPose(const AnimationPtr& anim, float time) { Entity::SetPose(anim, time); }

  Entity* Drawable::CopyTo(Entity* copyTo) const { return Entity::CopyTo(copyTo); }

  XmlNode* Drawable::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    ClearComponents();
    return Super::DeSerializeImp(info, parent);
  }

  XmlNode* Drawable::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  void Drawable::RemoveResources() { GetMeshManager()->Remove(GetMesh()->GetFile()); }

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
