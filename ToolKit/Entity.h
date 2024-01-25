/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

/**
 * @file Entity.h Header for Entity,
 */

#include "Animation.h"
#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "Node.h"
#include "Object.h"
#include "ToolKit.h"
#include "Types.h"

namespace ToolKit
{

  static VariantCategory EntityCategory {"Meta", 100};

  typedef std::unordered_map<ULongID, ComponentPtr> ComponentPtrMap;

  /**
   * Fundamental object that all the ToolKit utilities can interacted with.
   * Entity is the base class for all the objects that can be inserted in any
   * scene.
   */
  class TK_API Entity : public Object
  {
   public:
    TKDeclareClass(Entity, Object);

    Entity();
    virtual ~Entity();

    void NativeConstruct() override;

    EntityPtr Parent() const;
    virtual bool IsDrawable() const;
    virtual void SetPose(const AnimationPtr& anim, float time, BlendTarget* blendTarget = nullptr);
    virtual BoundingBox GetAABB(bool inWorld = false) const;
    ObjectPtr Copy() const override;
    virtual void RemoveResources();

    /**
     * Returns the visibility status of the current Entity. If it belongs to a
     * prefab, it returns the visibility of Prefab.
     */
    virtual bool IsVisible();

    void SetVisibility(bool vis, bool deep);
    void SetTransformLock(bool vis, bool deep);

    template <typename T>
    std::shared_ptr<T> AddComponent()
    {
      std::shared_ptr<T> component = MakeNewPtr<T>();
      component->OwnerEntity(Self<Entity>());
      m_components[T::StaticClass()->HashId] = component;
      return component;
    }

    void AddComponent(const ComponentPtr& componet);

    /**
     * Used to easily access first MeshComponentPtr.
     * @return First MeshComponentPtr if any otherwise empty pointer.
     */
    MeshComponentPtr GetMeshComponent() const;

    /**
     * Used to easily access first MaterialComponentPtr.
     * @return First MaterialComponentPtr if any exist, otherwise empty pointer.
     */
    MaterialComponentPtr GetMaterialComponent() const;

    /**
     * Remove the given component from the components of the Entity.
     * @param componentId Id of the component to be removed.
     * @return Removed ComponentPtr. If nothing gets removed, returns nullptr.
     */
    template <typename T>
    ComponentPtr RemoveComponent()
    {
      const auto& comp = m_components.find(T::StaticClass()->HashId);
      if (comp != m_components.end())
      {
        m_components.erase(comp);
        return comp->second;
      }

      return nullptr;
    }

    /**
     * Remove the given component from the components of the Entity.
     * @param Class is the class of the component to be removed.
     * @return Removed ComponentPtr. If nothing gets removed, returns nullptr.
     */
    ComponentPtr RemoveComponent(ClassMeta* Class);

    /**
     * Mutable component array accessors.
     * @return ComponentPtrArray for this Entity.
     */
    ComponentPtrMap& GetComponentPtrArray();

    /**
     * Immutable component array accessors.
     * @return ComponentPtrArray for this Entity.
     */
    const ComponentPtrMap& GetComponentPtrArray() const;

    /**
     * Used to return component of type T.
     * @return Component of type T if exist, otherwise nullptr.
     */
    template <typename T>
    std::shared_ptr<T> GetComponent() const
    {
      const auto& comp = m_components.find(T::StaticClass()->HashId);
      if (comp != m_components.cend())
      {
        return tk_reinterpret_pointer_cast<T>(comp->second);
      }

      return nullptr;
    }

    /**
     * Returns the component with the given class.
     * @return Component of type T if exist, otherwise empty pointer.
     */
    ComponentPtr GetComponent(ClassMeta* Class) const;

    /**
     * Removes all components from the entity.
     */
    void ClearComponents();

    /**
     * Used to identify if this Entity is a prefab, and if so, returns the
     * pointer to the parent prefab.
     * @return If the entity belongs to a Prefab it returns the pointer of the
     * prefab, otherwise it returns nullptr.
     */
    Entity* GetPrefabRoot() const;

   protected:
    virtual Entity* CopyTo(Entity* other) const;
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    void WeakCopy(Entity* other, bool copyComponents = true) const;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    XmlNode* DeSerializeImpV045(const SerializationFileInfo& info, XmlNode* parent);

   public:
    TKDeclareParam(String, Name);
    TKDeclareParam(String, Tag);
    TKDeclareParam(bool, Visible);
    TKDeclareParam(bool, TransformLock);

    /**
     * Node that holds the transform and parenting data for the Entity.
     */
    Node* m_node;

    /**
     * Internally used variable.
     * Helper ID for entity De serialization. Points to parent of the entity.
     */
    ULongID _parentId;

    /**
     * Internally used variable.
     * Used to indicate this entity belongs to a prefab entity. Set by the
     * Prefab Entity during Prefab::Init.
     */
    Entity* _prefabRootEntity;

   private:
    /**
     * Component map that may contains only one component per type.
     * It holds Class HashId - ComponentPtr
     */
    ComponentPtrMap m_components;
  };

  class TK_API EntityNode : public Entity
  {
   public:
    TKDeclareClass(EntityNode, Entity);

    EntityNode();
    explicit EntityNode(const String& name);
    virtual ~EntityNode();

    void RemoveResources() override;

   protected:
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
  };

  /**
   * DEPRECATED use ObjectFactory
   * Utility class to construct Entity.
   */
  class EntityFactory final
  {
   public:
    /**
     * Enums that shows the type of the Entity. Each derived class should provide
     * a type identifier for itself to make itself known to the ToolKit.
     */
    enum class EntityType
    {
      // Order is important. Don't change for backward comparable scene files.
      Entity_Base,
      Entity_AudioSource,
      Entity_Billboard,
      Entity_Cube,
      Entity_Quad,
      Entity_Sphere,
      Entity_Arrow,
      Entity_LineBatch,
      Entity_Cone,
      Entity_Drawable,
      Entity_SpriteAnim,
      Entity_Surface,
      Entity_Light,
      Entity_Camera,
      UNUSEDSLOT_1,
      Entity_Node,
      Entity_Button,
      Entity_Sky,
      Entity_DirectionalLight,
      Entity_PointLight,
      Entity_SpotLight,
      Entity_Canvas,
      Entity_Prefab,
      Entity_SkyBase,
      Entity_GradientSky,
      ENTITY_TYPE_COUNT // Holds the size of the enum
    };

   public:
    static EntityPtr CreateByType(EntityType type);
  };

} // namespace ToolKit
