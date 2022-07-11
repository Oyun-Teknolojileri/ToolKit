#pragma once

#include "Drawable.h"
#include "MathUtil.h"

namespace ToolKit
{
  namespace Editor
  {

    class Grid : public Entity
    {
     public:
      explicit Grid(UVec2 size);
      void Resize
      (
        UVec2 size,
        AxisLabel axis = AxisLabel::ZX,
        float gridSpaceScale = 1.0f
      );

      bool HitTest(const Ray& ray, Vec3& pos);

     public:
      UVec2 m_size;  // m^2 size of the grid.
      MaterialPtr m_material;
    };

  }  //  namespace Editor
}  //  namespace ToolKit
