#pragma once

/**
 * @file Entity.h Header for Entity,
 */

#include "Animation.h"
#include "Component.h"
#include "Node.h"
#include "ParameterBlock.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "Serialize.h"
#include "Types.h"

#include <memory>
#include <vector>

namespace ToolKit
{

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
    Etity_Arrow,
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

  static VariantCategory EntityCategory {"Meta", 100};

  /**
   * Fundamental object that all the ToolKit utilities can interacted with.
   * Entity is the base class for all the objects that can be inserted in any
   * scene.
   */
  class TK_API Entity : public Serializable
  {
   public:
    Entity();
    virtual ~Entity();

    static bool IsNotDrawable(Entity* ntt);
    bool ParentsVisible() const;
    virtual bool IsDrawable() const;
    virtual EntityType GetType() const;
    virtual void SetPose(const AnimationPtr& anim, float time);
    virtual BoundingBox GetAABB(bool inWorld = false) const;
    virtual Entity* Copy() const;
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent);
    virtual void RemoveResources();
    void SetVisibility(bool vis, bool deep);
    void SetTransformLock(bool vis, bool deep);
    bool IsSurfaceInstance() const;
    bool IsLightInstance() const;
    bool IsSkyInstance() const;

    /**
     * Adds the given component into the components of the Entity. While adding
     * the component to the list, function also converts the component to a
     * shared pointer for obtaining lifetime management. Also Entity set itself
     * as the new components parent.
     * @param component Component pointer that will be added to components of
     * this entity.
     */
    void AddComponent(Component* component);

    /**
     * Adds a component to the Entity same as AddComponent(Component*
     * component).
     */
    void AddComponent(ComponentPtr component);

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
    ComponentPtr RemoveComponent(ULongID componentId);

    ComponentPtrArray& GetComponentPtrArray();
    const ComponentPtrArray& GetComponentPtrArray() const;

    /**
     * Used to return first encountered component of type T.
     * @return First Component of type T if any exist, otherwise empty pointer.
     */
    template <typename T>
    std::shared_ptr<T> GetComponent() const
    {
      for (const ComponentPtr& com : GetComponentPtrArray())
      {
        if (com->GetType() == T::GetTypeStatic())
        {
          return std::reinterpret_pointer_cast<T>(com);
        }
      }

      return nullptr;
    }

    /**
     * Used to return all components of type T.
     * @param components ComponentPtrArray that will contain all encountered
     * components of type T.
     */
    template <typename T>
    void GetComponent(std::vector<std::shared_ptr<T>>& components) const
    {
      for (const ComponentPtr& com : GetComponentPtrArray())
      {
        if (com->GetType() == T::GetTypeStatic())
        {
          components.push_back(std::static_pointer_cast<T>(com));
        }
      }
    }

    /**
     * Used to return ComponentPtr with given id.
     * @param id Id of the Component that will be returned.
     * @return ComponentPtr with the given id.
     */
    ComponentPtr GetComponent(ULongID id) const;

    /**
     * Removes all components from the entity.
     */
    void ClearComponents();

    /**
     * If there is a material component, returns its material else returns
     * mesh's material. If there is not a MaterialComponent, it will return the
     * mesh's first material. In case of multisubmesh, there may be multiple
     * materials. But they are ignored.
     */
    MaterialPtr GetRenderMaterial() const;

   protected:
    virtual Entity* CopyTo(Entity* other) const;
    void ParameterConstructor();
    void WeakCopy(Entity* other, bool copyComponents = true) const;

   public:
    TKDeclareParam(ULongID, Id);
    TKDeclareParam(String, Name);
    TKDeclareParam(String, Tag);
    TKDeclareParam(bool, Visible);
    TKDeclareParam(bool, TransformLock);

    Node* m_node;
    ParameterBlock m_localData; // Entity's own data.

    /**
     * Internally used variable.
     * Helper ID for entity De serialization. Points to parent of the entity.
     */
    ULongID _parentId;

   private:
    // This should be private, because instantiated entities don't use this list
    // NOTE: Entity's own functions shouldn't access this either.
    //  They should use GetComponentPtrArray instead.
    ComponentPtrArray m_components;
  };

  class TK_API EntityNode : public Entity
  {
   public:
    EntityNode();
    explicit EntityNode(const String& name);
    virtual ~EntityNode();

    EntityType GetType() const override;
    void RemoveResources() override;
  };

  class TK_API EntityFactory
  {
   public:
    EntityFactory();
    ~EntityFactory();

    Entity* CreateByType(EntityType type);
    void OverrideEntityConstructor(EntityType type,
                                   std::function<Entity*()> fn);

   private:
    std::vector<std::function<Entity*()>> m_overrideFns;
  };

} // namespace ToolKit
