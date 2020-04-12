#pragma once

#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"

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
		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(-FLT_MAX);
  };

  struct Ray
  {
    glm::vec3 position;
    glm::vec3 direction;
  };

  struct PlaneEquation
  {
    glm::vec3 normal;
    float d;
  };

  struct Frustum
  {
    PlaneEquation planes[6]; // Left - Right - Top - Bottom - Near - Far
  };

  // Matrix Operations
  //////////////////////////////////////////

  void DecomposeMatrix(const glm::mat4& transform, glm::vec3& position, glm::quat& rotation, glm::vec3& scale); // Assuming transformation applied in this order translate * rotate * scale * vector.
  void DecomposeMatrix(const glm::mat4& transform, glm::vec3& position, glm::quat& rotation); // Assuming transformation applied in this order translate * rotate * scale * vector.
  void ExtractAxes(const glm::mat4& transform, glm::vec3& x, glm::vec3& y, glm::vec3& z, bool normalize = true);
  Frustum ExtractFrustum(const glm::mat4& projectViewModel);

  // Intersections
  //////////////////////////////////////////

  enum class IntersectResult
  {
    Outside,
    Inside,
    Intersect
  };

  bool SpherePointIntersection(const glm::vec3& spherePos, float sphereRadius, const glm::vec3& vertex);
  bool SphereSphereIntersection(const glm::vec3& spherePos, float sphereRadius, const glm::vec3& spherePos2, float sphereRadius2);
  bool RayBoxIntersection(const Ray& ray, const BoundingBox& box, float& t);
	bool RayTriangleIntersection(const Ray& ray, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t);
	bool RayMeshIntersection(class Mesh* const mesh, const Ray& ray, float& t);
  IntersectResult FrustumBoxIntersection(const Frustum& frustum, const BoundingBox& box); // 0 outside, 1 inside, 2 intersect
  bool RayPlaneIntersection(const Ray& ray, const PlaneEquation& plane, float& t);
  bool LinePlaneIntersection(const Ray& ray, const PlaneEquation& plane, float& t); // Line is same as ray but it is infinite on both sides. Unless ray is parallel to plane, it will always yield a result.
  glm::vec3 PointOnRay(const Ray& ray, float t);

  // Geometric Operations
  //////////////////////////////////////////
  void NormalzePlaneEquation(PlaneEquation& plane);
	void TransformAABB(BoundingBox& box, const glm::mat4& transform);
  PlaneEquation PlaneFrom(glm::vec3 const pnts[3]);
  PlaneEquation PlaneFrom(glm::vec3 point, glm::vec3 normal);

  // Conversions and Interpolation
  //////////////////////////////////////////
  glm::vec3 Interpolate(const glm::vec3& vec1, const glm::vec3& vec2, float ratio);
  void ToSpherical(glm::vec3 p, float& r, float& zenith, float& azimuth);
  glm::vec3 ToCartesian(float r, float zenith, float azimuth);
	glm::quat RotationTo(glm::vec3 a, glm::vec3 b); // Returns quaternion wich rotates a on to b.
}
