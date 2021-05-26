#include "stdafx.h"
#include "Entity.h"
#include "Node.h"
#include "ToolKit.h"
#include "Skeleton.h"
#include "MathUtil.h"
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

  Entity* Entity::GetCopy() const
  {
    Entity* cpy = new Entity();
    GetCopy(cpy);

    return cpy;
  }

  void Entity::GetCopy(Entity* copyTo) const
  {
    assert(copyTo->GetType() == GetType());
    SafeDel(copyTo->m_node);
    copyTo->m_node = m_node->GetCopy();
    copyTo->m_node->m_entity = copyTo;
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
    WriteAttr(node, doc, XmlEntityTypeAttr, std::to_string((int)GetType()));
    m_node->Serialize(doc, node);
  }

  void Entity::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    XmlNode* nttNode = nullptr;
    if (parent != nullptr)
    {
      nttNode = parent;
    }
    else
    {
      nttNode = doc->first_node(XmlEntityElement.c_str());
    }

    if (XmlAttribute* attr = nttNode->first_attribute(XmlEntityIdAttr.c_str()))
    {
      String val = attr->value();
      m_id = std::atoi(val.c_str());
    }

    if (XmlNode* transformNode = nttNode->first_node(XmlNodeElement.c_str()))
    {
      m_node->DeSerialize(doc, transformNode);
    }
  }

}
