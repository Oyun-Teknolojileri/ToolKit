#include "stdafx.h"
#include "Drawable.h"
#include "Mesh.h"
#include "Material.h"
#include "ToolKit.h"
#include "Node.h"
#include "Util.h"
#include "DebugNew.h"

namespace ToolKit
{

  Drawable::Drawable()
  {
    m_mesh = std::make_shared<Mesh>();
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
    if (m_mesh->IsSkinned())
    {
      Skeleton* skeleton = ((SkinMesh*)m_mesh.get())->m_skeleton;
      anim->GetCurrentPose(skeleton);
    }
    else
    {
      anim->GetCurrentPose(m_node);
    }
  }

  BoundingBox Drawable::GetAABB(bool inWorld) const
  {
    BoundingBox bb = m_mesh->m_aabb;
    if (inWorld)
    {
      TransformAABB(bb, m_node->GetTransform(TransformationSpace::TS_WORLD));
    }

    return bb;
  }

  Entity* Drawable::GetCopy(Entity* copyTo) const
  {
    Entity::GetCopy(copyTo);
    Drawable* ntt = static_cast<Drawable*> (copyTo);
    ntt->m_mesh = m_mesh->Copy<Mesh>();
    return ntt;
  }

  void Drawable::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Entity::Serialize(doc, parent);
    XmlNode* node = doc->allocate_node(rapidxml::node_element, XmlMeshElement.c_str());

    String relPath = GetRelativeResourcePath(m_mesh->GetFile());
    
    node->append_attribute
    (
      doc->allocate_attribute
      (
        XmlFileAttr.c_str(),
        doc->allocate_string(relPath.c_str())
      )
    );
    parent->last_node()->append_node(node);
    m_mesh->Save(true);
  }

  void Drawable::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
    if (XmlNode* meshNode = parent->first_node(XmlMeshElement.c_str()))
    {
      XmlAttribute* attr = meshNode->first_attribute(XmlFileAttr.c_str());
      String filePath = attr->value();
      m_mesh = GetMeshManager()->Create<Mesh>(MeshPath(filePath));
    }
  }

  bool Drawable::IsMaterialInUse(const MaterialPtr& mat) const
  {
    if (mat)
    {
      MeshRawCPtrArray meshes;
      m_mesh->GetAllMeshes(meshes);
      for (const Mesh* m : meshes)
      {
        if (mat == m->m_material)
        {
          return true;
        }
      }
    }

    return false;
  }

  bool Drawable::IsMeshInUse(const MeshPtr& mesh) const
  {
    if (mesh)
    {
      return m_mesh == mesh;
    }

    return false;
  }

  void Drawable::RemoveResources()
  {
    GetMeshManager()->Remove(m_mesh->GetFile());
  }

  Entity* Drawable::GetInstance(Entity* copyTo) const
  {
    Entity::GetInstance(copyTo);
    Drawable* instance = dynamic_cast<Drawable*> (copyTo);
    instance->m_mesh = m_mesh;

    return instance;
  }

}
