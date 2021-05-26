#pragma once

#include "Drawable.h"
#include "MathUtil.h"

namespace ToolKit
{

  namespace Editor
  {
    class Grid : public Drawable
    {
    public:
      Grid(uint size);
      void Resize(uint size, float gridSpaceScale = 1.0f);
      bool HitTest(const Ray& ray, Vec3& pos);

    public:
      uint m_size; // m^2 size of the grid.
      MaterialPtr m_material;
    };
  }
}