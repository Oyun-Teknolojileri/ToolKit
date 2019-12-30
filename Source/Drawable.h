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
    virtual EntityType GetType();
    virtual bool IsDrawable();
    virtual void SetPose(Animation* anim);

  public:
    std::shared_ptr<Mesh> m_mesh;
  };

}