#include "Entity.h"

#include "Light.h"
#include "MathUtil.h"
#include "Node.h"
#include "Prefab.h"
#include "ResourceComponent.h"
#include "Skeleton.h"
#include "Sky.h"
#include "ToolKit.h"
#include "Util.h"

#include "DebugNew.h"

namespace ToolKit
{

  Entity::Entity()
  {
    ParameterConstructor();

    m_node           = new Node();
    m_node->m_entity = this;
    _parentId        = 0;
  }

  Entity::~Entity()
  {
    SafeDel(m_node);
    ClearComponents();
  }

  bool Entity::IsDrawable() const
  {
    return GetComponent<MeshComponent>() != nullptr;
  }

  EntityType Entity::GetType() const
  {
    return EntityType::Entity_Base;
  }

  void Entity::SetPose(const AnimationPtr& anim, float time)
  {
    MeshComponentPtr meshComp = GetMeshComponent();
    if (meshComp)
    {
      MeshPtr mesh                  = meshComp->GetMeshVal();
      SkeletonComponentPtr skelComp = GetComponent<SkeletonComponent>();
      if (mesh->IsSkinned() && skelComp)
      {
        anim->GetPose(skelComp, time);
        return;
      }
    }
    anim->GetPose(m_node, time);
  }

  BoundingBox Entity::GetAABB(bool inWorld) const
  {
    BoundingBox aabb;

    AABBOverrideComponentPtr overrideComp =
        GetComponent<AABBOverrideComponent>();
    if (overrideComp)
    {
      aabb = overrideComp->GetAABB();
    }
    else
    {
      MeshComponentPtrArray meshCmps;
      GetComponent<MeshComponent>(meshCmps);

      if (meshCmps.empty())
      {
        // Unit aabb.
        aabb.max = Vec3(0.5f, 0.5f, 0.5f);
        aabb.min = Vec3(-0.5f, -0.5f, -0.5f);
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
    Entity* e    = GetEntityFactory()->CreateByType(t);
    return CopyTo(e);
  }

  void Entity::ClearComponents()
  {
    // Probably base entity will call this too, so there is no problem to use
    //  like that
    m_components.clear();
  }

  MaterialPtr Entity::GetRenderMaterial() const
  {
    MaterialPtr renderMat = nullptr;
    if (MaterialComponentPtr matCom = GetMaterialComponent())
    {
      renderMat = matCom->GetMaterialVal();
    }
    else if (MeshComponentPtr meshCom = GetMeshComponent())
    {
      renderMat = meshCom->GetMeshVal()->m_material;
    }

    if (renderMat == nullptr)
    {
      assert(false && "Entity with no material.");
      renderMat = GetMaterialManager()->GetCopyOfDefaultMaterial();
    }

    return renderMat;
  }

  Entity* Entity::CopyTo(Entity* other) const
  {
    WeakCopy(other);

    other->ClearComponents();
    for (const ComponentPtr& com : GetComponentPtrArray())
    {
      other->GetComponentPtrArray().push_back(com->Copy(other));
    }

    return other;
  }

  void Entity::ParameterConstructor()
  {
    m_localData.m_variants.reserve(6);
    ULongID id = GetHandleManager()->GetNextHandle();

    Id_Define(id, EntityCategory.Name, EntityCategory.Priority, true, false);

    Name_Define("Entity_" + std::to_string(id),
                EntityCategory.Name,
                EntityCategory.Priority,
                true,
                true);

    Tag_Define("", EntityCategory.Name, EntityCategory.Priority, true, true);

    Visible_Define(
        true, EntityCategory.Name, EntityCategory.Priority, true, true);

    TransformLock_Define(
        false, EntityCategory.Name, EntityCategory.Priority, true, true);
  }

  void Entity::WeakCopy(Entity* other, bool copyComponents) const
  {
    assert(other->GetType() == GetType());
    SafeDel(other->m_node);
    other->m_node           = m_node->Copy();
    other->m_node->m_entity = other;

    // Preserve Ids.
    ULongID id         = other->GetIdVal();
    other->m_localData = m_localData;
    other->SetIdVal(id);

    if (copyComponents)
    {
      other->GetComponentPtrArray() = GetComponentPtrArray();
    }
  }

  void Entity::AddComponent(Component* component)
  {
    AddComponent(ComponentPtr(component));
  }

  void Entity::AddComponent(ComponentPtr component)
  {
    assert(GetComponent(component->m_id) == nullptr &&
           "Component has already been added.");

    component->m_entity = this;
    GetComponentPtrArray().push_back(component);
  }

  MeshComponentPtr Entity::GetMeshComponent() const
  {
    return GetComponent<MeshComponent>();
  }

  MaterialComponentPtr Entity::GetMaterialComponent() const
  {
    return GetComponent<MaterialComponent>();
  }

  ComponentPtr Entity::RemoveComponent(ULongID componentId)
  {
    ComponentPtrArray& compList = GetComponentPtrArray();
    for (size_t i = 0; i < compList.size(); i++)
    {
      ComponentPtr com = compList[i];
      if (com->m_id == componentId)
      {
        compList.erase(compList.begin() + i);
        return com;
      }
    }

    return nullptr;
  }

  ComponentPtrArray& Entity::GetComponentPtrArray()
  {
    return m_components;
  }

  const ComponentPtrArray& Entity::GetComponentPtrArray() const
  {
    return m_components;
  }

  ComponentPtr Entity::GetComponent(ULongID id) const
  {
    for (const ComponentPtr& com : GetComponentPtrArray())
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
    WriteAttr(node, doc, XmlEntityIdAttr, std::to_string(GetIdVal()));
    if (m_node->m_parent && m_node->m_parent->m_entity)
    {
      WriteAttr(node,
                doc,
                XmlParentEntityIdAttr,
                std::to_string(m_node->m_parent->m_entity->GetIdVal()));
    }

    WriteAttr(node,
              doc,
              XmlEntityTypeAttr,
              std::to_string(static_cast<int>(GetType())));

    m_node->Serialize(doc, node);
    m_localData.Serialize(doc, node);

    XmlNode* compNode = CreateXmlNode(doc, "Components", node);
    for (const ComponentPtr& cmp : GetComponentPtrArray())
    {
      cmp->Serialize(doc, compNode);
    }
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

    ClearComponents();
    if (XmlNode* components = node->first_node("Components"))
    {
      XmlNode* comNode = components->first_node(XmlComponent.c_str());
      while (comNode)
      {
        int type = -1;
        ReadAttr(comNode, XmlParamterTypeAttr, type);
        Component* com =
            Component::CreateByType(static_cast<ComponentType>(type));

        com->DeSerialize(doc, comNode);
        AddComponent(com);

        comNode = comNode->next_sibling();
      }
    }
  }

  void Entity::RemoveResources()
  {
    assert(false && "Not implemented");
  }

  void Entity::SetVisibility(bool vis, bool deep)
  {
    SetVisibleVal(vis);
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
    SetTransformLockVal(lock);
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

  bool Entity::IsSurfaceInstance() const
  {
    switch (GetType())
    {
    case EntityType::Entity_Surface:
    case EntityType::Entity_Button:
    case EntityType::Entity_Canvas:
      return true;
    default:
      return false;
    }
  }

  bool Entity::IsLightInstance() const
  {
    const EntityType type = GetType();
    return (type == EntityType::Entity_Light ||
            type == EntityType::Entity_DirectionalLight ||
            type == EntityType::Entity_PointLight ||
            type == EntityType::Entity_SpotLight);
  }

  bool Entity::IsSkyInstance() const
  {
    const EntityType type = GetType();
    return (type == EntityType::Entity_Sky ||
            type == EntityType::Entity_SkyBase ||
            type == EntityType::Entity_GradientSky);
  }

  EntityNode::EntityNode()
  {
  }

  EntityNode::EntityNode(const String& name)
  {
    SetNameVal(name);
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

  EntityFactory::EntityFactory()
  {
    m_overrideFns.resize(static_cast<size_t>(EntityType::ENTITY_TYPE_COUNT),
                         nullptr);
  }

  EntityFactory::~EntityFactory()
  {
    m_overrideFns.clear();
  }

  Entity* EntityFactory::CreateByType(EntityType type)
  {
    // Overriden constructors
    if (m_overrideFns[static_cast<int>(type)] != nullptr)
    {
      return m_overrideFns[static_cast<int>(type)]();
    }

    Entity* e = nullptr;
    switch (type)
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
    case EntityType::Entity_DirectionalLight:
      e = new DirectionalLight();
      break;
    case EntityType::Entity_PointLight:
      e = new PointLight();
      break;
    case EntityType::Entity_SpotLight:
      e = new SpotLight();
      break;
    case EntityType::Entity_Sky:
      e = new Sky();
      break;
    case EntityType::Entity_GradientSky:
      e = new GradientSky();
      break;
    case EntityType::Entity_Canvas:
      e = new Canvas();
      break;
    case EntityType::Entity_Prefab:
      e = new Prefab();
      break;
    case EntityType::Entity_SpriteAnim:
    case EntityType::UNUSEDSLOT_1:
    default:
      assert(false);
      break;
    }

    return e;

    return nullptr;
  }

  void EntityFactory::OverrideEntityConstructor(EntityType type,
                                                std::function<Entity*()> fn)
  {
    m_overrideFns[static_cast<int>(type)] = fn;
  }

} // namespace ToolKit
