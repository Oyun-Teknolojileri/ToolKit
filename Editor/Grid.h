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
      float m_gridCellSize = 1.0f;  // m^2 size of each cell
      Vec3 m_horizontalAxisColor = Vec3(1.0f, 0.0f, 0.0f);
      Vec3 m_verticalAxisColor = Vec3(0.0f, 0.0f, 1.0f);
      MaterialPtr m_material;
    };

  }  //  namespace Editor
}  //  namespace ToolKit
