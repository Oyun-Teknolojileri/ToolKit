#include "stdafx.h"
#include "Entity.h"
#include "Node.h"
#include "ToolKit.h"
#include "Skeleton.h"
#include "MathUtil.h"
#include "Util.h"
#include "DebugNew.h"

namespace ToolKit
{

  EntityId Entity::m_lastId = 1; // 0 is null entity id.

  Entity::Entity()
  {
    m_node = new Node();
    m_node->m_entity = this;
    m_id = m_lastId++;
    m_name = "Entity_" + std::to_string(m_id);
    _parentId = 0;
  }

  Entity::~Entity()
  {
    SafeDel(m_node);
  }

  bool Entity::IsDrawable() const
  {
    return false;
  }

  EntityType Entity::GetType() const
  {
    return EntityType::Entity_Base;
  }

  void Entity::SetPose(Animation* anim)
  {
    anim->GetCurrentPose(m_node);
  }

  struct BoundingBox Entity::GetAABB(bool inWorld) const
  {
    BoundingBox aabb;

    // Unit aabb.
    aabb.min = Vec3(0.5f, 0.5f, 0.5f);
    aabb.max = Vec3(-0.5f, -0.5f, -0.5f);

    if (inWorld)
    {
      TransformAABB(aabb, m_node->GetTransform(TransformationSpace::TS_WORLD));
    }

    return aabb;
  }

  Entity* Entity::Copy() const
  {
    EntityType t = GetType();
    Entity* e = CreateByType(t);
    return GetCopy(e);
  }

  Entity* Entity::GetCopy(Entity* copyTo) const
  {
    // This just copies the node, other contents should be copied by the descendent.
    assert(copyTo->GetType() == GetType());
    SafeDel(copyTo->m_node);
    copyTo->m_node = m_node->Copy();
    copyTo->m_node->m_entity = copyTo;
    copyTo->m_name = m_name;
    copyTo->m_tag = m_tag;

    return copyTo;
  }

  Entity* Entity::GetInstance() const
  {
    EntityType t = GetType();
    Entity* e = CreateByType(t);
    return GetInstance(e);
  }

  Entity* Entity::GetInstance(Entity* copyTo) const
  {
    return GetCopy(copyTo);
  }

  Entity* Entity::CreateByType(EntityType t)
  {
    Entity* e = nullptr;
    switch (t)
    {
    case EntityType::Entity_Base:
      e = new Entity();
      break;
    case EntityType::Entity_Node:
      e = new EntityNode();
      break;
    case EntityType::Entity_AudioSource:
      e = new AudioSource();
      break;
    case EntityType::Entity_Billboard:
      e = new Billboard(Billboard::Settings());
      break;
    case EntityType::Entity_Cube:
      e = new Cube(false);
      break;
    case EntityType::Entity_Quad:
      e = new Quad(false);
      break;
    case EntityType::Entity_Sphere:
      e = new Sphere(false);
      break;
    case EntityType::Etity_Arrow:
      e = new Arrow2d(false);
      break;
    case EntityType::Entity_LineBatch:
      e = new LineBatch();
      break;
    case EntityType::Entity_Cone:
      e = new Cone(false);
      break;
    case EntityType::Entity_Drawable:
      e = new Drawable();
      break;
    case EntityType::Entity_Camera:
      e = new Camera();
      break;
    case EntityType::Entity_SpriteAnim:
    case EntityType::Entity_Surface:
    case EntityType::Entity_Light:
    case EntityType::Entity_Directional:
    default:
      assert(false);
      break;
    }

    return e;
  }

  void Entity::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* node = doc->allocate_node(rapidxml::node_element, XmlEntityElement.c_str());
    if (parent != nullptr)
    {
      parent->append_node(node);
    }
    else
    {
      doc->append_node(node);
    }

    WriteAttr(node, doc, XmlEntityIdAttr, std::to_string(m_id));
    if (m_node->m_parent && m_node->m_parent->m_entity)
    {
      WriteAttr(node, doc, XmlParentEntityIdAttr, std::to_string(m_node->m_parent->m_entity->m_id));
    }
    
    WriteAttr(node, doc, XmlEntityNameAttr, m_name);
    WriteAttr(node, doc, XmlEntityTagAttr, m_tag);
    WriteAttr(node, doc, XmlEntityTypeAttr, std::to_string((int)GetType()));
    m_node->Serialize(doc, node);
  }

  void Entity::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    XmlNode* node = nullptr;
    if (parent != nullptr)
    {
      node = parent;
    }
    else
    {
      node = doc->first_node(XmlEntityElement.c_str());
    }

    ReadAttr(node, XmlEntityIdAttr, *(uint*)&m_id);
    ReadAttr(node, XmlParentEntityIdAttr, *(uint*)&_parentId);
    ReadAttr(node, XmlEntityNameAttr, m_name);
    ReadAttr(node, XmlEntityTagAttr, m_tag);

    if (XmlNode* transformNode = node->first_node(XmlNodeElement.c_str()))
    {
      m_node->DeSerialize(doc, transformNode);
    }
  }

  void Entity::RemoveResources()
  {
    assert(false && "Not implemented");
  }

}
