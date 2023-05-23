/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

/**
 * @file
 * This file contains declarations for various geometric shapes and related
 * utilities.
 */

#include "Types.h"

namespace ToolKit
{

  /**
   * A templated struct for a rectangle with X, Y, Width and Height.
   * @tparam T The type of the rectangle's components.
   */
  template <class T>
  struct Rect
  {
    T X;      //!< The X-coordinate of the rectangle.
    T Y;      //!< The Y-coordinate of the rectangle.
    T Width;  //!< The width of the rectangle.
    T Height; //!< The height of the rectangle.
  };

  /**
   * A typedef for an integer-based rectangle.
   */
  typedef Rect<int> IRectangle;

  /**
   *  A typedef for a float-based rectangle.
   */
  typedef Rect<float> FRectangle;

  /**
   *  A struct representing a bounding box in 3D space.
   */
  struct BoundingBox
  {
    Vec3 min = Vec3(TK_FLT_MAX);  //!< The minimum point of the bounding box.
    Vec3 max = Vec3(-TK_FLT_MAX); //!< The maximum point of the bounding box.

    /**
     * Check if the bounding box is valid.
     * @return True if the bounding box is valid, false otherwise.
     */
    bool IsValid() const { return !glm::isinf(Volume()); }

    /**
     * Get the center point of the bounding box.
     * @return The center point of the bounding box.
     */
    Vec3 GetCenter() const { return min + (max - min) * 0.5f; }

    /**
     * Update the boundary of the bounding box with a new point.
     * @param v The new point to include in the bounding box.
     */
    void UpdateBoundary(const Vec3& v)
    {
      max = glm::max(max, v);
      min = glm::min(min, v);
    }

    /**
     * Update the boundary of the bounding box with another bounding box.
     * @param bb The bounding box to include in the bounding box.
     */
    void UpdateBoundary(const BoundingBox& bb)
    {
      UpdateBoundary(bb.max);
      UpdateBoundary(bb.min);
    }

    /**
     * Get the volume of the bounding box.
     * @return The volume of the bounding box.
     */
    float Volume() const { return glm::abs((max.x - min.x) * (max.y - min.y) * (max.z - min.z)); }

    /**
     * Get the width of the bounding box.
     * @return The width of the bounding box.
     */
    float GetWidth() const { return max.x - min.x; }

    /**
     * Get the height of the bounding box.
     * @return The height of the bounding box.
     */
    float GetHeight() const { return max.y - min.y; }
  };

  /**
   * A struct representing a ray in 3D space.
   */
  struct Ray
  {
    Vec3 position;  //!< The position of the ray.
    Vec3 direction; //!< The direction of the ray.
  };

  /**
   * A struct representing a plane equation in 3D space.
   */
  struct PlaneEquation
  {
    Vec3 normal; //!< The normal vector of the plane.
    float d;     //!< The distance from the origin to the plane along the normal.
  };

  /**
   * A struct representing a frustum in 3D space.
   */
  struct Frustum
  {
    PlaneEquation planes[6]; // Left - Right - Top - Bottom - Near - Far
  };

  /**
   * A struct representing a bounding sphere in 3D space.
   */
  struct BoundingSphere
  {
    Vec3 pos;     ///< The position of the center of the sphere.
    float radius; ///< The radius of the sphere.
  };

} // namespace ToolKit