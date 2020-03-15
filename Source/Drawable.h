#pragma once

#include "Entity.h"
#include <memory>

namespace ToolKit
{

  class Mesh;
  class Texture;

  class Drawable : public Entity
  {
  public:
    Drawable();
    virtual ~Drawable();
		virtual bool IsDrawable() override;
    virtual EntityType GetType() override;
    virtual void SetPose(Animation* anim) override;
		virtual struct BoundingBox GetAABB(bool inWorld = false) override;

  public:
    std::shared_ptr<Mesh> m_mesh;
  };

}