#pragma once

#include "ToolKit.h"
#include "Drawable.h"
#include "MathUtil.h"
#include "Resource.h"
#include <vector>

namespace ToolKit
{

  class Surface : public Drawable
  {
  public:
    Surface(TexturePtr texture, const Vec2& pivotOffset);
    Surface(TexturePtr texture, const SpriteEntry& entry);
    Surface(const String& textureFile, const Vec2& pivotOffset);
    virtual ~Surface();

    virtual EntityType GetType() const override;

  private:
    void AssignTexture();
    void CreateQuat();
    void CreateQuat(const SpriteEntry& val);

  private:
    Vec2 m_pivotOffset;
  };

}