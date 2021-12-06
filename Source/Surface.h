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
    Surface();
    Surface(TexturePtr texture, const Vec2& pivotOffset);
    Surface(TexturePtr texture, const SpriteEntry& entry);
    Surface(const String& textureFile, const Vec2& pivotOffset);
    Surface(const Vec2& size, const Vec2& offset = { 0.5f, 0.5f });
    virtual ~Surface();

    virtual EntityType GetType() const override;

  protected:
    virtual Entity* GetCopy(Entity* copyTo) const override;

  private:
    void AssignTexture();
    void CreateQuat();
    void CreateQuat(const SpriteEntry& val);
    void SetSizeFromTexture();

  public:
    Vec2 m_size;
    Vec2 m_pivotOffset;
  };

}