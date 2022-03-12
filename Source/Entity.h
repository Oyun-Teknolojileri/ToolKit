#pragma once

#include "Types.h"
#include "ParameterBlock.h"
#include "Component.h"

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

  class TK_API Entity
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
    void SetVisibility(bool vis, bool deep);
    bool IsSurfaceInstance();

    static Entity* CreateByType(EntityType t);

    // Component functionalities.
    void AddComponent(Component* component);
    void GetComponent(ComponentType type, ComponentArray& components) const;
    Component* GetComponent(ULongID id) const;
    Component* GetFirstComponent(ComponentType type) const;

  protected:
    virtual Entity* GetCopy(Entity* copyTo) const;
    virtual Entity* GetInstance(Entity* copyTo) const;

  public:
    Node* m_node;
    ULongID m_id;
    String m_name;
    String m_tag;
    bool m_visible;
    ParameterBlock m_customData;
    ComponentArray m_components;

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
