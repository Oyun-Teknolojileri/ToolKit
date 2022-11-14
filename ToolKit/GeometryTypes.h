#pragma once

#include "Types.h"

namespace ToolKit
{

  template <class T>
  struct Rect
  {
    T X;
    T Y;
    T Width;
    T Height;
  };

  typedef Rect<int> IRectangle;
  typedef Rect<float> FRectangle;

  struct BoundingBox
  {
    Vec3 min = Vec3(TK_FLT_MAX);
    Vec3 max = Vec3(-TK_FLT_MAX);

    Vec3 GetCenter()
    {
      return min + (max - min) * 0.5f;
    }

    void UpdateBoundary(const Vec3& v)
    {
      max = glm::max(max, v);
      min = glm::min(min, v);
    }

    float Volume()
    {
      return glm::abs((max.x - min.x) * (max.y - min.y) * (max.z - min.z));
    }

    float GetWidth() const
    {
      return max.x - min.x;
    }

    float GetHeight() const
    {
      return max.y - min.y;
    }
  };

  struct Ray
  {
    Vec3 position;
    Vec3 direction;
  };

  struct PlaneEquation
  {
    Vec3 normal;
    float d;
  };

  struct Frustum
  {
    PlaneEquation planes[6]; // Left - Right - Top - Bottom - Near - Far
  };

  struct BoundingSphere
  {
    Vec3 pos;
    float radius;
  };

}