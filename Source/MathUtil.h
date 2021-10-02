#pragma once

#include "Types.h"

namespace ToolKit
{
  // Geometric types
  //////////////////////////////////////////
  template <class T>
  struct Rect
  {
    T x;
    T y;
    T width;
    T height;
  };

  struct BoundingBox
  {
    Vec3 min = Vec3(FLT_MAX);
    Vec3 max = Vec3(-FLT_MAX);

    Vec3 GetCenter()
    {
      return min + (max - min) * 0.5f;
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

  // Matrix Operations
  //////////////////////////////////////////

  void DecomposeMatrix(const Mat4& transform, Vec3* translation, Quaternion* orientation, Vec3* scale);
  bool IsAffine(const Mat4& transform);
  void QDUDecomposition(const Mat3& transform, Mat3& kQ, Vec3& kD, Vec3& kU);
  void ExtractAxes(const Mat4& transform, Vec3& x, Vec3& y, Vec3& z, bool normalize = true);
  Frustum ExtractFrustum(const Mat4& projectViewModel);

  // Intersections
  //////////////////////////////////////////

  enum class IntersectResult
  {
    Outside,
    Inside,
    Intersect
  };

  bool SpherePointIntersection(const Vec3& spherePos, float sphereRadius, const Vec3& vertex);
  bool SphereSphereIntersection(const Vec3& spherePos, float sphereRadius, const Vec3& spherePos2, float sphereRadius2);
  bool RayBoxIntersection(const Ray& ray, const BoundingBox& box, float& t);
  bool RayTriangleIntersection(const Ray& ray, const Vec3& v0, const Vec3& v1, const Vec3& v2, float& t);
  bool RayMeshIntersection(class Mesh* const mesh, const Ray& ray, float& t);
  IntersectResult FrustumBoxIntersection(const Frustum& frustum, const BoundingBox& box); // 0 outside, 1 inside, 2 intersect
  bool RayPlaneIntersection(const Ray& ray, const PlaneEquation& plane, float& t);
  bool RaySphereIntersection(const Ray& ray, const BoundingSphere& sphere, float& t);
  bool LinePlaneIntersection(const Ray& ray, const PlaneEquation& plane, float& t); // Line is same as ray but it is infinite on both sides. Unless ray is parallel to plane, it will always yield a result.
  Vec3 PointOnRay(const Ray& ray, float t);

  // Geometric Operations
  //////////////////////////////////////////
  void NormalizePlaneEquation(PlaneEquation& plane);
  void TransformAABB(BoundingBox& box, const Mat4& transform);
  void GetCorners(const BoundingBox& box, Vec3Array& corners);
  PlaneEquation PlaneFrom(Vec3 const pnts[3]);
  PlaneEquation PlaneFrom(Vec3 point, Vec3 normal);
  float SignedDistance(const PlaneEquation& plane, const Vec3& pnt);
  Vec3 ProjectPointOntoPlane(const PlaneEquation& plane, const Vec3& pnt);

  // Conversions and Interpolation
  //////////////////////////////////////////
  Vec3 Interpolate(const Vec3& vec1, const Vec3& vec2, float ratio);
  void ToSpherical(Vec3 p, float& r, float& zenith, float& azimuth);
  Vec3 ToCartesian(float r, float zenith, float azimuth);
  Quaternion RotationTo(Vec3 a, Vec3 b); // Returns quaternion wich rotates a on to b.

  // Comparison
  //////////////////////////////////////////
  template<typename T>
  bool VecAllEqual(const T& a, const T& b)
  {
    return glm::all(glm::equal<T>(a, b));
  }

  // Numberic operations
  //////////////////////////////////////////
  template<typename T>
  T SetPrecision(const T& val, int nDecimal);
}
