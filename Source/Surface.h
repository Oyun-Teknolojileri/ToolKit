#pragma once

#include "ToolKit.h"
#include "Drawable.h"
#include "MathUtil.h"
#include "Resource.h"
#include <vector>

namespace ToolKit
{

  class Vertex;

  class Surface : public Drawable, public Resource
  {
  public:
    Surface(TexturePtr texture, Vec2 pivotOffset);
    Surface(TexturePtr texture, const SpriteEntry& entry);
    Surface(String file, Vec2 pivotOffset);
    ~Surface();

    virtual EntityType GetType() const override;
    virtual void Load() override;
    virtual void Init(bool flushClientSideArray = true) override;
		virtual void UnInit() override;

  private:
    void CreateQuat();
    void CreateQuat(const SpriteEntry& val);

  private:
    Vec2 m_pivotOffset;
  };

}