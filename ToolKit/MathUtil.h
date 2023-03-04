#pragma once

#include "GeometryTypes.h"
#include "Types.h"

namespace ToolKit
{

  // Matrix Operations
  //////////////////////////////////////////

  TK_API void DecomposeMatrix(const Mat4& transform,
                              Vec3* translation,
                              Quaternion* orientation,
                              Vec3* scale);

  TK_API bool IsAffine(const Mat4& transform);

  TK_API void QDUDecomposition(const Mat3& transform,
                               Mat3& kQ,
                               Vec3& kD,
                               Vec3& kU);

  TK_API void ExtractAxes(const Mat4& transform,
                          Vec3& x,
                          Vec3& y,
                          Vec3& z,
                          bool normalize = true);

  TK_API Frustum ExtractFrustum(const Mat4& projectViewModel, bool normalize);

  // Intersections
  //////////////////////////////////////////

  enum class IntersectResult
  {
    Outside,
    Inside,
    Intersect
  };

  TK_API float SquareDistancePointToAABB(const Vec3& p, const BoundingBox& b);

  TK_API bool SphereBoxIntersection(const BoundingSphere& s,
                                    const BoundingBox& b);

  TK_API bool SpherePointIntersection(const Vec3& spherePos,
                                      float sphereRadius,
                                      const Vec3& vertex);

  TK_API bool SphereSphereIntersection(const Vec3& spherePos,
                                       float sphereRadius,
                                       const Vec3& spherePos2,
                                       float sphereRadius2);

  TK_API bool BoxBoxIntersection(const BoundingBox& box1,
                                 const BoundingBox& box2);

  TK_API bool BoxPointIntersection(const BoundingBox& box, const Vec3& point);

  TK_API bool RayBoxIntersection(const Ray& ray,
                                 const BoundingBox& box,
                                 float& t);

  TK_API bool RayTriangleIntersection(const Ray& ray,
                                      const Vec3& v0,
                                      const Vec3& v1,
                                      const Vec3& v2,
                                      float& t);

  TK_API Vec3 CPUSkinning(const class SkinVertex* vertex,
                          const Skeleton* skel,
                          const class DynamicBoneMap* dynamicBoneMap);

  class SkeletonComponent;
  TK_API bool RayMeshIntersection(const class Mesh* const mesh,
                                  const Ray& rayInWorldSpace,
                                  float& t,
                                  const SkeletonComponent* skelComp = nullptr);

  // @return TK_UINT_MAX = no intersection, otherwise submesh index
  // If there is no tracing possible object, t set as 0.0
  // Tracing possible object: Vertex positions should be in memory
  TK_API uint FindMeshIntersection(const class Entity* const ntt,
                                   const Ray& ray,
                                   float& t);

  TK_API IntersectResult FrustumBoxIntersection(
      const Frustum& frustum,
      const BoundingBox& box); // 0 outside, 1 inside, 2 intersect

  TK_API bool RayPlaneIntersection(const Ray& ray,
                                   const PlaneEquation& plane,
                                   float& t);

  TK_API bool RaySphereIntersection(const Ray& ray,
                                    const BoundingSphere& sphere,
                                    float& t);

  // Line is same as ray but it is infinite on both sides.
  // Unless ray is parallel to plane, it will always yield a result.
  TK_API bool LinePlaneIntersection(const Ray& ray,
                                    const PlaneEquation& plane,
                                    float& t);

  TK_API Vec3 PointOnRay(const Ray& ray, float t);

  // Geometric Operations
  //////////////////////////////////////////

  TK_API void NormalizePlaneEquation(PlaneEquation& plane);
  TK_API void TransformAABB(BoundingBox& box, const Mat4& transform);
  TK_API void GetCorners(const BoundingBox& box, Vec3Array& corners);
  TK_API PlaneEquation PlaneFrom(const Vec3 pnts[3]);
  TK_API PlaneEquation PlaneFrom(Vec3 point, Vec3 normal);
  TK_API float SignedDistance(const PlaneEquation& plane, const Vec3& pnt);
  TK_API Vec3 ProjectPointOntoPlane(const PlaneEquation& plane, const Vec3& pt);
  TK_API Vec3 ProjectPointOntoLine(const Ray& ray, const Vec3& pnt);

  /**
   * Removes the entities that are outside of the camera.
   * @param entities All entities.
   * @param camera Camera that is being used for generating frustum.
   */
  TK_API void FrustumCull(EntityRawPtrArray& entities, Camera* camera);
  TK_API void FrustumCull(RenderJobArray& jobs, Camera* camera);

  // Conversions and Interpolation
  //////////////////////////////////////////

  TK_API Vec3 Interpolate(const Vec3& vec1, const Vec3& vec2, float ratio);
  TK_API void ToSpherical(Vec3 p, float& r, float& zenith, float& azimuth);
  TK_API Vec3 ToCartesian(float r, float zenith, float azimuth);
  // Returns quaternion wich rotates a on to b.
  TK_API Quaternion RotationTo(Vec3 a, Vec3 b);
  // Returns an orthogonal vector to v.
  TK_API Vec3 Orthogonal(const Vec3& v);

  // Comparison
  //////////////////////////////////////////

  template <typename T>
  bool VecAllEqual(const T& a, const T& b)
  {
    return glm::all(glm::equal<T>(a, b));
  }

  TK_API bool PointInsideBBox(const Vec3& point,
                              const Vec3& max,
                              const Vec3& min);

  // Random Generators
  //////////////////////////////////////////
  
  /**
   * Generate random points on a hemisphere with proximity to normal.
   * @params numSamples number of samples to generate.
   * @params k distribution bias of the samples. 0 all samples on normal of the
   * hemisphere. 1 totally uniformly distributed random samples.
   * @returns Generated samples.
   */
  Vec3Array GenerateHemispherePoints(int numSamples, float k);


} // namespace ToolKit
