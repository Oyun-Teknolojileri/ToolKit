#include "stdafx.h"
#include "Drawable.h"
#include "Mesh.h"
#include "Material.h"
#include "ToolKit.h"
#include "Node.h"
#include "Util.h"
#include "Component.h"
#include "DebugNew.h"

namespace ToolKit
{

  Drawable::Drawable()
  {
    MeshComponent* mc = new MeshComponent();
    mc->m_mesh = std::make_shared<Mesh>();
    AddComponent(mc);
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

  void Drawable::SetPose(Animation* anim)
  {
    MeshPtr mesh = GetMesh();
    if (mesh->IsSkinned())
    {
      Skeleton* skeleton = ((SkinMesh*)mesh.get())->m_skeleton;
      anim->GetCurrentPose(skeleton);
    }
    else
    {
      anim->GetCurrentPose(m_node);
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
    XmlNode* node = doc->allocate_node(rapidxml::node_element, XmlMeshElement.c_str());

    String relPath = GetRelativeResourcePath(GetMesh()->GetSerializeFile());
    node->append_attribute
    (
      doc->allocate_attribute
      (
        XmlFileAttr.c_str(),
        doc->allocate_string(relPath.c_str())
      )
    );

    parent->last_node()->append_node(node);
    GetMesh()->Save(true);
  }

  void Drawable::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
    if (XmlNode* meshNode = parent->first_node(XmlMeshElement.c_str()))
    {
      XmlAttribute* attr = meshNode->first_attribute(XmlFileAttr.c_str());
      String filePath = attr->value();
      MeshPtr mesh = GetMeshManager()->Create<Mesh>(MeshPath(filePath));
      SetMesh(mesh);
    }
  }

  void Drawable::RemoveResources()
  {
    GetMeshManager()->Remove(GetMesh()->GetFile());
  }

  MeshPtr& Drawable::GetMesh() const
  {
    MeshComponentPtr meshComp = GetComponent<MeshComponent>();
    return meshComp->m_mesh;
  }

  void Drawable::SetMesh(const MeshPtr& mesh)
  {
    MeshComponentPtr meshComp = GetComponent<MeshComponent>();
    meshComp->m_mesh = mesh;
  }

}
