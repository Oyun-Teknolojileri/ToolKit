#pragma once

#include "Types.h"

namespace ToolKit
{
  class Node;
  class Animation;

  enum class EntityType
  {
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
    Entity_Node
  };

  class Entity
  {
  public:
    Entity();
    virtual ~Entity();

    virtual bool IsDrawable() const;
    virtual EntityType GetType() const;
    virtual void SetPose(Animation* anim);
    virtual struct BoundingBox GetAABB(bool inWorld = false) const;
    virtual Entity* Copy() const;
    virtual Entity* GetInstance() const;
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent);
    virtual void RemoveResources();

    static Entity* CreateByType(EntityType t);

  protected:
    virtual Entity* GetCopy(Entity* copyTo) const;
    virtual Entity* GetInstance(Entity* copyTo) const;

  public:
    Node* m_node;
    EntityId m_id;
    String m_name;
    String m_tag;

    // Internal use only, Helper ID for entity deserialization.
    EntityId _parentId;

  private:
    static EntityId m_lastId;
  };

  class EntityNode : public Entity
  {
  public:
    EntityNode() {}
    virtual ~EntityNode() {}
    virtual EntityType GetType() const override { return EntityType::Entity_Node; }
    virtual void RemoveResources() override {}
  };

}
