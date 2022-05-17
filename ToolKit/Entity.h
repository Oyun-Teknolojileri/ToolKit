#pragma once

#include <memory>
#include <vector>

#include "Types.h"
#include "ParameterBlock.h"
#include "Component.h"
#include "Node.h"
#include "Animation.h"
#include "Serialize.h"

namespace ToolKit
{

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
    Entity_Directional,
    Entity_Node,
    Entity_Button
  };

  class TK_API Entity : public Serializable
  {
   public:
    Entity();
    virtual ~Entity();

    virtual bool IsDrawable() const;
    virtual EntityType GetType() const;
    virtual void SetPose(const AnimationPtr& anim, float time);
    virtual struct BoundingBox GetAABB(bool inWorld = false) const;
    virtual Entity* Copy() const;
    virtual Entity* Instantiate() const;
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent);
    virtual void RemoveResources();
    void SetVisibility(bool vis, bool deep);
    void SetTransformLock(bool vis, bool deep);
    bool IsSurfaceInstance();

    static Entity* CreateByType(EntityType t);

    // Component functionalities.
    void AddComponent(Component* component);

    template<typename T>
    std::shared_ptr<T> GetComponent() const
    {
      for (const ComponentPtr& com : m_components)
      {
        if (com->GetType() == T::GetTypeStatic())
        {
          return std::reinterpret_pointer_cast<T> (com);
        }
      }

      return nullptr;
    }

    template<typename T>
    void GetComponent(std::vector<std::shared_ptr<T>>& components) const
    {
      for (const ComponentPtr& com : m_components)
      {
        if (com->GetType() == T::GetTypeStatic())
        {
          components.push_back(std::static_pointer_cast<T> (com));
        }
      }
    }

    ComponentPtr GetComponent(ULongID id) const;

   protected:
    virtual Entity* CopyTo(Entity* other) const;
    virtual Entity* InstantiateTo(Entity* other) const;
    void ParameterConstructor();
    void WeakCopy(Entity* other, bool copyComponents = true) const;

   public:
    TKDeclareParam(ULongID, Id);
    TKDeclareParam(String, Name);
    TKDeclareParam(String, Tag);
    TKDeclareParam(bool, Visible);
    TKDeclareParam(bool, TransformLock);

    Node* m_node;
    ParameterBlock m_localData;  // Entity's own data.
    ComponentPtrArray m_components;

    /**
    * Internally used variable.
    * Helper ID for entity De serialization. Points to parent of the entity.
    */
    ULongID _parentId;
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

}  // namespace ToolKit

