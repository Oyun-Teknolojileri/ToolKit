/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Entity.h"

#include "Audio.h"
#include "Camera.h"
#include "Canvas.h"
#include "GradientSky.h"
#include "Light.h"
#include "MathUtil.h"
#include "Node.h"
#include "Prefab.h"
#include "ResourceComponent.h"
#include "Skeleton.h"
#include "Sky.h"
#include "Surface.h"
#include "ToolKit.h"
#include "Util.h"

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(Entity, TKObject);

  Entity::Entity()
  {
    m_node           = new Node();
    m_node->m_entity = this;
    _parentId        = 0;
  }

  Entity::~Entity()
  {
    SafeDel(m_node);
    ClearComponents();
  }

  bool Entity::IsDrawable() const { return GetComponent<MeshComponent>() != nullptr; }

  EntityType Entity::GetType() const { return EntityType::Entity_Base; }

  void Entity::SetPose(const AnimationPtr& anim, float time, BlendTarget* blendTarget)
  {
    MeshComponentPtr meshComp = GetMeshComponent();
    if (meshComp)
    {
      MeshPtr mesh                  = meshComp->GetMeshVal();
      SkeletonComponentPtr skelComp = GetComponent<SkeletonComponent>();
      if (mesh->IsSkinned() && skelComp)
      {
        anim->GetPose(skelComp, time, blendTarget);
        return;
      }
    }
    anim->GetPose(m_node, time);
  }

  BoundingBox Entity::GetAABB(bool inWorld) const
  {
    BoundingBox aabb;

    AABBOverrideComponentPtr overrideComp = GetComponent<AABBOverrideComponent>();
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

  Entity* Entity::GetPrefabRoot() const { return _prefabRootEntity; }

  Entity* Entity::CopyTo(Entity* other) const
  {
    WeakCopy(other);

    other->ClearComponents();
    for (const ComponentPtr& com : m_components)
    {
      other->m_components.push_back(com->Copy(other));
    }

    return other;
  }

  void Entity::ParameterConstructor()
  {
    Super::ParameterConstructor();

    Name_Define(StaticClass()->Name, EntityCategory.Name, EntityCategory.Priority, true, true);
    Tag_Define("", EntityCategory.Name, EntityCategory.Priority, true, true);
    Visible_Define(true, EntityCategory.Name, EntityCategory.Priority, true, true);
    TransformLock_Define(false, EntityCategory.Name, EntityCategory.Priority, true, true);
  }

  void Entity::ParameterEventConstructor() { Super::ParameterEventConstructor(); }

  void Entity::WeakCopy(Entity* other, bool copyComponents) const
  {
    assert(other->GetType() == GetType());
    SafeDel(other->m_node);
    other->m_node           = m_node->Copy();
    other->m_node->m_entity = other;

    // Preserve Ids.
    ULongID id              = other->GetIdVal();
    other->m_localData      = m_localData;
    other->SetIdVal(id);

    if (copyComponents)
    {
      other->m_components = m_components;
    }
  }

  void Entity::AddComponent(const ComponentPtr& component)
  {
    assert(GetComponent(component->GetIdVal()) == nullptr && "Component has already been added.");

    component->m_entity = this;
    m_components.push_back(component);
  }

  MeshComponentPtr Entity::GetMeshComponent() const { return GetComponent<MeshComponent>(); }

  MaterialComponentPtr Entity::GetMaterialComponent() const { return GetComponent<MaterialComponent>(); }

  ComponentPtr Entity::RemoveComponent(ULongID componentId)
  {
    for (size_t i = 0; i < m_components.size(); i++)
    {
      ComponentPtr com = m_components[i];
      if (com->GetIdVal() == componentId)
      {
        m_components.erase(m_components.begin() + i);
        return com;
      }
    }

    return nullptr;
  }

  ComponentPtrArray& Entity::GetComponentPtrArray() { return m_components; }

  const ComponentPtrArray& Entity::GetComponentPtrArray() const { return m_components; }

  ComponentPtr Entity::GetComponent(ULongID id) const
  {
    for (const ComponentPtr& com : GetComponentPtrArray())
    {
      if (com->GetIdVal() == id)
      {
        return com;
      }
    }

    return nullptr;
  }

  XmlNode* Entity::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* objNode = Super::SerializeImp(doc, parent);
    XmlNode* node    = CreateXmlNode(doc, StaticClass()->Name, objNode);

    if (m_node->m_parent && m_node->m_parent->m_entity)
    {
      WriteAttr(node, doc, XmlParentEntityIdAttr, std::to_string(m_node->m_parent->m_entity->GetIdVal()));
    }

    m_node->Serialize(doc, node);

    XmlNode* compNode = CreateXmlNode(doc, XmlComponentArrayElement, node);
    for (const ComponentPtr& cmp : GetComponentPtrArray())
    {
      cmp->Serialize(doc, compNode);
    }

    return node;
  }

  void Entity::DeSerializeImp(XmlDocument* doc, XmlNode* parent)
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
        Component* com = GetComponentFactory()->Create((ComponentType) type);
        com->DeSerialize(doc, comNode);
        AddComponent(std::shared_ptr<Component>(com));

        comNode = comNode->next_sibling();
      }
    }
  }

  void Entity::RemoveResources() { assert(false && "Not implemented"); }

  bool Entity::IsVisible()
  {
    if (Entity* root = GetPrefabRoot())
    {
      // If parent is not visible, all objects must be hidden.
      // Otherwise, prefer to use its value.
      if (root->GetVisibleVal() == false)
      {
        return false;
      }
    }

    return GetVisibleVal();
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
    return (type == EntityType::Entity_Light || type == EntityType::Entity_DirectionalLight ||
            type == EntityType::Entity_PointLight || type == EntityType::Entity_SpotLight);
  }

  bool Entity::IsSkyInstance() const
  {
    const EntityType type = GetType();
    return (type == EntityType::Entity_Sky || type == EntityType::Entity_SkyBase ||
            type == EntityType::Entity_GradientSky);
  }

  EntityNode::EntityNode() {}

  EntityNode::EntityNode(const String& name) { SetNameVal(name); }

  EntityNode::~EntityNode() {}

  EntityType EntityNode::GetType() const { return EntityType::Entity_Node; }

  void EntityNode::RemoveResources() {}

  XmlNode* EntityNode::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  EntityFactory::EntityFactory() { m_overrideFns.resize(static_cast<size_t>(EntityType::ENTITY_TYPE_COUNT), nullptr); }

  EntityFactory::~EntityFactory() { m_overrideFns.clear(); }

  Entity* EntityFactory::CreateByType(EntityType type)
  {
    // Overriden constructors
    if (m_overrideFns[(int) type] != nullptr)
    {
      Entity* ntt = m_overrideFns[(int) type]();
      ntt->NativeConstruct();

      return ntt;
    }

    Entity* ntt = nullptr;
    switch (type)
    {
    case EntityType::Entity_Base:
      ntt = new Entity();
      break;
    case EntityType::Entity_Node:
      ntt = new EntityNode();
      break;
    case EntityType::Entity_AudioSource:
      ntt = new AudioSource();
      break;
    case EntityType::Entity_Billboard:
      ntt = new Billboard(Billboard::Settings());
      break;
    case EntityType::Entity_Cube:
      ntt = new Cube();
      break;
    case EntityType::Entity_Quad:
      ntt = new Quad();
      break;
    case EntityType::Entity_Sphere:
      ntt = new Sphere(false);
      break;
    case EntityType::Entity_Arrow:
      ntt = new Arrow2d(false);
      break;
    case EntityType::Entity_LineBatch:
      ntt = new LineBatch();
      break;
    case EntityType::Entity_Cone:
      ntt = new Cone(false);
      break;
    case EntityType::Entity_Drawable:
      ntt = new Drawable();
      break;
    case EntityType::Entity_Camera:
      ntt = new Camera();
      break;
    case EntityType::Entity_Surface:
      ntt = new Surface();
      break;
    case EntityType::Entity_Button:
      ntt = new Button();
      break;
    case EntityType::Entity_Light:
      ntt = new Light();
      break;
    case EntityType::Entity_DirectionalLight:
      ntt = new DirectionalLight();
      break;
    case EntityType::Entity_PointLight:
      ntt = new PointLight();
      break;
    case EntityType::Entity_SpotLight:
      ntt = new SpotLight();
      break;
    case EntityType::Entity_Sky:
      ntt = new Sky();
      break;
    case EntityType::Entity_GradientSky:
      ntt = new GradientSky();
      break;
    case EntityType::Entity_Canvas:
      ntt = new Canvas();
      break;
    case EntityType::Entity_Prefab:
      ntt = new Prefab();
      break;
    case EntityType::Entity_SpriteAnim:
    case EntityType::UNUSEDSLOT_1:
    default:
      assert(false);
      break;
    }

    ntt->NativeConstruct();

    return ntt;
  }

  TKDefineClass(EntityNode, Entity);

  void EntityFactory::OverrideEntityConstructor(EntityType type, std::function<Entity*()> fn)
  {
    m_overrideFns[static_cast<int>(type)] = fn;
  }

} // namespace ToolKit
