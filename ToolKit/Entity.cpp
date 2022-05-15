#include "Entity.h"
#include "Node.h"
#include "ToolKit.h"
#include "Skeleton.h"
#include "MathUtil.h"
#include "Util.h"
#include "DebugNew.h"
#include "Light.h"

namespace ToolKit
{

  Entity::Entity()
  {
    ParameterConstructor();

    m_node = new Node();
    m_node->m_entity = this;
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

  void Entity::ParameterConstructor()
  {
    m_localData.m_variants.reserve(6);
    ULongID id = GetHandleManager()->GetNextHandle();

    Id_Define(id, "Meta", 100, true, false);
    Name_Define("Entity_" + std::to_string(id), "Meta", 100, true, true);
    Tag_Define("Tag", "Meta", 100, true, true);
    Visible_Define(true, "Meta", 100, true, true);
    TransformLock_Define(false, "Meta", 100, true, true);
  }

  void Entity::WeakCopy(Entity* other, bool copyComponents) const
  {
    assert(other->GetType() == GetType());
    SafeDel(other->m_node);
    other->m_node = m_node->Copy();
    other->m_node->m_entity = other;

    // Preserve Ids.
    ULongID id = other->Id();
    other->m_localData = m_localData;
    other->Id() = id;

    if (copyComponents)
    {
      other->m_components = m_components;
    }
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
      case EntityType::Entity_Light:
      e = new Light();
      break;
      case EntityType::Entity_SpriteAnim:
      case EntityType::Entity_Directional:
      default:
      assert(false);
      break;
    }

    return e;
  }

  void Entity::AddComponent(Component* component)
  {
    assert
    (
      GetComponent(component->m_id) == nullptr &&
      "Component has already been added."
    );

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
    WriteAttr(node, doc, XmlEntityIdAttr, std::to_string(IdC()));
    if (m_node->m_parent && m_node->m_parent->m_entity)
    {
      WriteAttr
      (
        node,
        doc,
        XmlParentEntityIdAttr,
        std::to_string(m_node->m_parent->m_entity->IdC())
      );
    }

    WriteAttr
    (
      node,
      doc,
      XmlEntityTypeAttr,
      std::to_string(static_cast<int> (GetType()))
    );

    m_node->Serialize(doc, node);
    m_localData.Serialize(doc, node);
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

    ReadAttr(node, XmlParentEntityIdAttr, _parentId);

    if (XmlNode* transformNode = node->first_node(XmlNodeElement.c_str()))
    {
      m_node->DeSerialize(doc, transformNode);
    }

    m_localData.DeSerialize(doc, parent);
  }

  void Entity::RemoveResources()
  {
    assert(false && "Not implemented");
  }

  void Entity::SetVisibility(bool vis, bool deep)
  {
    Visible() = vis;
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

  void Entity::SetTransformLock(bool lock, bool deep)
  {
    TransformLock() = lock;
    if (deep)
    {
      EntityRawPtrArray children;
      GetChildren(this, children);
      for (Entity* c : children)
      {
        c->SetTransformLock(lock, true);
      }
    }
  }

  bool Entity::IsSurfaceInstance()
  {
    EntityType t = GetType();
    return
      t == EntityType::Entity_Surface ||
      t == EntityType::Entity_Button;
  }

  EntityNode::EntityNode()
  {
  }

  EntityNode::EntityNode(const String& name)
  {
    Name() = name;
  }

  EntityNode::~EntityNode()
  {
  }

  EntityType EntityNode::GetType() const
  {
    return EntityType::Entity_Node;
  }

  void EntityNode::RemoveResources()
  {
  }

}  // namespace ToolKit

