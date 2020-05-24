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
		virtual bool IsDrawable() const override;
    virtual EntityType GetType() const override;
    virtual void SetPose(Animation* anim) override;
		virtual struct BoundingBox GetAABB(bool inWorld = false) const override;
    virtual Drawable* GetCopy() const override;
    virtual void GetCopy(Entity* copyTo) const override;

  public:
    std::shared_ptr<Mesh> m_mesh;
  };

}