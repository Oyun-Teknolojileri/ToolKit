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
    Entity_Cube,
    Entity_Quad,
    Entity_Sphere,
    Etity_Arrow,
    Entity_LineBatch,
    Entity_Drawable,
    Entity_SpriteAnim,
    Entity_Surface,
    Entity_Directional,
    Entity_Light,
    Entity_Camera,
  };

  class Entity
  {
  public:
    Entity();
    virtual ~Entity();

    virtual bool IsDrawable();
    virtual EntityType GetType();
    virtual void SetPose(Animation* anim);
		virtual struct BoundingBox GetAABB(bool inWorld = false);

  public:
    Node* m_node;
		EntityId m_id;

	private:
		static EntityId m_lastId;
  };

}
