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
    Entity_Actor,
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
    Entity_Directional
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

  public:
    Node* m_node;
		EntityId m_id;

	private:
		static EntityId m_lastId;
  };

}
