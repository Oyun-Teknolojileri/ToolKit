/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "GeometryTypes.h"

namespace ToolKit
{

  // Matrix Operations
  //////////////////////////////////////////

  TK_API void DecomposeMatrix(const Mat4& transform, Vec3* translation, Quaternion* orientation, Vec3* scale);

  TK_API bool IsAffine(const Mat4& transform);

  TK_API void QDUDecomposition(const Mat3& transform, Mat3& kQ, Vec3& kD, Vec3& kU);

  /** Extracts the basis vectors from the transform matrix. */
  TK_API void ExtractAxes(const Mat4& transform, Vec3& x, Vec3& y, Vec3& z, bool normalize = true);

  /*
   * Extracts a frustum from given matrix whose plane normals are pointing inwards.
   *
   * If the given matrix is projection matrix, then the algorithm gives the
   * clipping planes in view space If the matrix is projection * view,then the
   * algorithm gives the clipping planes in world space If the matrix is
   * projection * view * model, then the algorithm gives the clipping planes in
   * model space
   *
   * @param projectViewModel is the matrix to extract frustum from.
   * @param normalize if set true normalizes the frustum matrices.
   *
   * @return Frustum that is extracted from given matrix.
   */
  TK_API Frustum ExtractFrustum(const Mat4& projectViewModel, bool normalize);

  /** Normalizes frustum's planes. */
  TK_API void NormalizeFrustum(Frustum& frustum);

  // Intersections
  //////////////////////////////////////////

  enum class IntersectResult
  {
    Outside,
    Inside,
    Intersect
  };

  TK_API float SquareDistancePointToAABB(const Vec3& p, const BoundingBox& b);

  TK_API bool SphereBoxIntersection(const BoundingSphere& s, const BoundingBox& b);

  TK_API bool SpherePointIntersection(const Vec3& spherePos, float sphereRadius, const Vec3& vertex);

  TK_API bool SphereSphereIntersection(const Vec3& spherePos,
                                       float sphereRadius,
                                       const Vec3& spherePos2,
                                       float sphereRadius2);

  TK_API IntersectResult BoxBoxIntersection(const BoundingBox& box1, const BoundingBox& box2);

  TK_API bool BoxPointIntersection(const BoundingBox& box, const Vec3& point);

  TK_API bool RayBoxIntersection(const Ray& ray, const BoundingBox& box, float& t);

  TK_API bool RayEntityIntersection(const Ray& ray, const EntityPtr entity, float& dist);

  TK_API bool RectPointIntersection(Vec2 rectMin, Vec2 rectMax, Vec2 point);

  TK_API bool RayTriangleIntersection(const Ray& ray, const Vec3& v0, const Vec3& v1, const Vec3& v2, float& t);

  TK_API Vec3 CPUSkinning(const class SkinVertex* vertex,
                          const Skeleton* skel,
                          DynamicBoneMapPtr dynamicBoneMap,
                          bool isAnimated);

  TK_API bool RayMeshIntersection(const class Mesh* const mesh,
                                  const Ray& rayInWorldSpace,
                                  float& t,
                                  const SkeletonComponentPtr skelComp = nullptr);

  /**
   * For both skinned and mesh objects, finds the ray mesh intersection.
   * If there are sub meshes, in case of a hit returns the sub mesh index and intersection distance t.
   * If there is no intersection, returns TK_UINT_MAX and sets t to TK_FLT_MAX.
   */
  TK_API uint FindMeshIntersection(const EntityPtr ntt, const Ray& ray, float& t);

  TK_API IntersectResult FrustumBoxIntersection(const Frustum& frustum, const BoundingBox& box);

  TK_API bool ConePointIntersection(Vec3 conePos, Vec3 coneDir, float coneHeight, float coneAngle, Vec3 point);

  TK_API bool FrustumConeIntersect(const Frustum& frustum,
                                   Vec3 conePos,
                                   Vec3 coneDir,
                                   float coneHeight,
                                   float coneAngle);

  /** Tests normalized frustum with a sphere. */
  TK_API bool FrustumSphereIntersection(const Frustum& frustum, const Vec3& pos, float radius);

  /** Tests normalized frustum with a sphere. */
  TK_API bool FrustumSphereIntersection(const Frustum& frustum, const BoundingSphere& sphere);

  /** Tests a ray with a plane if there is an intersection sets the t as distance from ray to plane. */
  TK_API bool RayPlaneIntersection(const Ray& ray, const PlaneEquation& plane, float& t);

  TK_API bool RaySphereIntersection(const Ray& ray, const BoundingSphere& sphere, float& t);

  /** Finds a point at t distance away from the ray. */
  TK_API Vec3 PointOnRay(const Ray& ray, float t);

  /** Tests whether the given point is inside the given boundary. */
  TK_API bool PointInsideBBox(const Vec3& point, const Vec3& max, const Vec3& min);

  // Geometric Operations
  //////////////////////////////////////////

  /** Transforms the 8 corners of the bounding box and recalculates new boundary from transformed points. */
  TK_API void TransformAABB(BoundingBox& box, const Mat4& transform);

  /** Calculates 8 corners of the given bounding box. */
  TK_API void GetCorners(const BoundingBox& box, Vec3Array& corners);

  /** Normalize the plane equation by normalizing its normal and dividing normal's magnitude by the distance d. */
  TK_API void NormalizePlaneEquation(PlaneEquation& plane);

  /** Construct a plane equation in the form of ax+by+cz+(-d)=0 */
  TK_API PlaneEquation PlaneFrom(const Vec3 pnts[3]);

  /** Construct a plane equation in the form of ax+by+cz+(-d)=0 */
  TK_API PlaneEquation PlaneFrom(Vec3 point, Vec3 normal);

  /** Tests the box against the frustum and returns true if its intersects or inside. */
  TK_API bool FrustumTest(const Frustum& frustum, const BoundingBox& box);

  /**
   * Removes the entities that are outside of the camera.
   * @param entities All entities.
   * @param camera Camera that is being used for generating frustum.
   */
  TK_API void FrustumCull(EntityRawPtrArray& entities, const CameraPtr& camera);
  TK_API void FrustumCull(RenderJobArray& jobs, const CameraPtr& camera);
  TK_API void FrustumCull(const RenderJobArray& jobs, const CameraPtr& camera, UIntArray& resultIndices);
  TK_API void FrustumCull(const RenderJobArray& jobs, const CameraPtr& camera, RenderJobArray& unCulledJobs);

  // Conversions and Interpolation
  //////////////////////////////////////////

  /** Linearly interpolate vec1 with vec2 based on the ratio. Formula: (v2-v1)*ratio+v1 */
  TK_API Vec3 Interpolate(const Vec3& vec1, const Vec3& vec2, float ratio);

  /** From the given point, calculates radius (r), angle from y axis (zenith), angle in xz plane (azimuth) */
  TK_API void ToSpherical(Vec3 p, float& r, float& zenith, float& azimuth);

  /** From given spherical coordinates calculates a point in Cartesian space. */
  TK_API Vec3 ToCartesian(float r, float zenith, float azimuth);

  /** Returns quaternion wich rotates a on to b. */
  TK_API Quaternion RotationTo(Vec3 a, Vec3 b);

  /** Returns an orthogonal vector to v. */
  TK_API Vec3 Orthogonal(const Vec3& v);

  // Comparison
  //////////////////////////////////////////

  /** Tests if all components of given vectors are equal or not. */
  template <typename T>
  bool VecAllEqual(const T& a, const T& b)
  {
    return glm::all(glm::equal<T>(a, b));
  }

  // Random Generators
  //////////////////////////////////////////

  /**
   * Generate random points on a hemisphere with proximity to normal.
   * @params numSamples number of samples to generate.
   * @params bias divergence from the hemisphere normal.0 all samples on normal
   * of the hemisphere. 1 totally uniformly distributed random samples.
   * @returns Generated samples.
   */
  Vec3Array GenerateRandomSamplesOnHemisphere(int numSamples, float bias);

  /**
   * Same as GenerateRandomSamplesOnHemisphere() however this version allows
   * samples to be inside the hemisphere.
   */
  void GenerateRandomSamplesInHemisphere(int numSamples, float bias, Vec3Array& array);

} // namespace ToolKit
