#pragma once

#include "Types.h"
#include "ParameterBlock.h"
#include "Component.h"
#include "Serialize.h"

namespace ToolKit
{
  class Node;
  class Animation;

  enum class EntityType
  {
    Entity_Base, // Order is important. Don't change for backward compatable scene files.
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
    virtual void SetPose(Animation* anim);
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
    virtual void ParameterConstructor();

  private:
    void WeakCopy(Entity* other) const;

  public:
    Node* m_node;
    ULongID m_id;
    String m_name;
    String m_tag;
    bool m_visible;
    bool m_transformLocked;
    ParameterBlock m_localData; // Entitie's own data.
    ParameterBlock m_customData; // Users can define variables from editor.
    ComponentPtrArray m_components;

    // Internal use only, Helper ID for entity deserialization.
    ULongID _parentId;

  private:
    static ULongID m_lastId;
  };

  class TK_API EntityNode : public Entity
  {
  public:
    EntityNode() {}
    EntityNode(const String& name) { m_name = name; }
    virtual ~EntityNode() {}
    virtual EntityType GetType() const override { return EntityType::Entity_Node; }
    virtual void RemoveResources() override {}
  };

}
