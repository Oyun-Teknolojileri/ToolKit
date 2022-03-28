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

  ULongID Entity::m_lastId = 1000; // 0 is null entity id. Preserve previous ID slots for internal use.

  Entity::Entity()
  {
    m_node = new Node();
    m_node->m_entity = this;
    m_id = m_lastId++;
    m_name = "Entity_" + std::to_string(m_id);
    m_visible = true;
    _parentId = 0;
  }

  Entity::~Entity()
  {
    SafeDel(m_node);
    m_components.clear();
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

    MeshComponentPtrArray meshCmps;
    GetComponent<MeshComponent>(meshCmps);

    if (meshCmps.empty())
    {
      // Unit aabb.
      aabb.min = Vec3(0.5f, 0.5f, 0.5f);
      aabb.max = Vec3(-0.5f, -0.5f, -0.5f);
    }
    else
    {
      BoundingBox cmpAABB;
      for (MeshComponentPtr& cmp : meshCmps)
      {
        cmpAABB = cmp->GetAABB();
        aabb.UpdateBoundary(cmpAABB.max);
        aabb.UpdateBoundary(cmpAABB.min);
      }
    }

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
    return CopyTo(e);
  }

  Entity* Entity::CopyTo(Entity* other) const
  {
    WeakCopy(other);

    other->m_components.clear();
    for (const ComponentPtr& com : m_components)
    {
      other->m_components.push_back(com->Copy());
    }

    return other;
  }

  Entity* Entity::Instantiate() const
  {
    EntityType t = GetType();
    Entity* e = CreateByType(t);
    return InstantiateTo(e);
  }

  Entity* Entity::InstantiateTo(Entity* other) const
  {
    WeakCopy(other);
    return other;
  }

  void Entity::WeakCopy(Entity* other) const
  {
    assert(other->GetType() == GetType());
    SafeDel(other->m_node);
    other->m_node = m_node->Copy();
    other->m_node->m_entity = other;
    other->m_name = m_name;
    other->m_tag = m_tag;
    other->m_visible = m_visible;
    other->m_customData = m_customData;
    other->m_components = m_components;
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
    case EntityType::Entity_Surface:
      e = new Surface();
      break;
    case EntityType::Entity_Button:
      e = new Button();
      break;
    case EntityType::Entity_SpriteAnim:
    case EntityType::Entity_Light:
    case EntityType::Entity_Directional:
    default:
      assert(false);
      break;
    }

    return e;
  }

  void Entity::AddComponent(Component* component)
  {
    assert(GetComponent(component->m_id) == nullptr && "Component has already been added.");
    m_components.push_back(ComponentPtr(component));
  }

  ComponentPtr Entity::GetComponent(ULongID id) const
  {
    for (const ComponentPtr& com : m_components)
    {
      if (com->m_id == id)
      {
        return com;
      }
    }

    return nullptr;
  }

  void Entity::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* node = CreateXmlNode(doc, XmlEntityElement, parent);
    WriteAttr(node, doc, XmlEntityIdAttr, std::to_string(m_id));
    if (m_node->m_parent && m_node->m_parent->m_entity)
    {
      WriteAttr(node, doc, XmlParentEntityIdAttr, std::to_string(m_node->m_parent->m_entity->m_id));
    }
    
    WriteAttr(node, doc, XmlEntityNameAttr, m_name);
    WriteAttr(node, doc, XmlEntityTagAttr, m_tag);
    WriteAttr(node, doc, XmlEntityTypeAttr, std::to_string((int)GetType()));
    WriteAttr(node, doc, XmlEntityVisAttr, std::to_string(m_visible));
    m_node->Serialize(doc, node);
    m_customData.Serialize(doc, node);
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
    ReadAttr(node, XmlEntityVisAttr, m_visible);

    if (XmlNode* transformNode = node->first_node(XmlNodeElement.c_str()))
    {
      m_node->DeSerialize(doc, transformNode);
    }
    m_customData.DeSerialize(doc, parent);
  }

  void Entity::RemoveResources()
  {
    assert(false && "Not implemented");
  }

  void Entity::SetVisibility(bool vis, bool deep)
  {
    m_visible = vis;
    if (deep)
    {
      EntityRawPtrArray children;
      GetChildren(this, children);
      for (Entity* c : children)
      {
        c->SetVisibility(vis, true);
      }
    }
  }

  bool Entity::IsSurfaceInstance()
  {
    EntityType t = GetType();
    return t == EntityType::Entity_Surface ||
      t == EntityType::Entity_Button;
  }

}
