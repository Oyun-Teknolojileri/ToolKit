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

#include "MathUtil.h"

#include "Camera.h"
#include "Mesh.h"
#include "Node.h"
#include "Pass.h"
#include "ResourceComponent.h"
#include "Skeleton.h"

#include <execution>
#include <mutex>

#include "DebugNew.h"

namespace ToolKit
{

  void DecomposeMatrix(const Mat4& transform, Vec3* translation, Quaternion* orientation, Vec3* scale)
  {
    // assert(IsAffine(transform));

    if (scale != nullptr || orientation != nullptr)
    {
      Mat3 matQ;
      Vec3 s, vecU;
      QDUDecomposition(transform, matQ, s, vecU);

      if (scale != nullptr)
      {
        *scale = s;
      }

      if (orientation != nullptr)
      {
        *orientation = glm::toQuat(matQ);
      }
    }

    if (translation != nullptr)
    {
      *translation = glm::column(transform, 3).xyz;
    }
  }

  bool IsAffine(const Mat4& transform)
  {
    return transform[0][3] == 0 && transform[1][3] == 0 && transform[2][3] == 0 && transform[3][3] == 1;
  }

  // https://github.com/OGRECave/ogre-next/blob/master/OgreMain/src/OgreMatrix3.cpp
  void QDUDecomposition(const Mat3& transform, Mat3& kQ, Vec3& kD, Vec3& kU)
  {
    // Factor M = QR = QDU where Q is orthogonal, D is diagonal,
    // and U is upper triangular with ones on its diagonal.  Algorithm uses
    // Gram-Schmidt orthogonalization (the QR algorithm).
    //
    // If M = [ m0 | m1 | m2 ] and Q = [ q0 | q1 | q2 ], then
    //
    //   q0 = m0/|m0|
    //   q1 = (m1-(q0*m1)q0)/|m1-(q0*m1)q0|
    //   q2 = (m2-(q0*m2)q0-(q1*m2)q1)/|m2-(q0*m2)q0-(q1*m2)q1|
    //
    // where |V| indicates length of vector V and A*B indicates dot
    // product of vectors A and B.  The matrix R has entries
    //
    //   r00 = q0*m0  r01 = q0*m1  r02 = q0*m2
    //   r10 = 0      r11 = q1*m1  r12 = q1*m2
    //   r20 = 0      r21 = 0      r22 = q2*m2
    //
    // so D = diag(r00,r11,r22) and U has entries u01 = r01/r00,
    // u02 = r02/r00, and u12 = r12/r11.

    // Q = rotation
    // D = scaling
    // U = shear

    // D stores the three diagonal entries r00, r11, r22
    // U stores the entries U[0] = u01, U[1] = u02, U[2] = u12

    // build orthogonal matrix Q
    // TK_MOD To row major. Ogre is row major.
    Mat4 m           = glm::transpose(transform);
    float fInvLength = glm::inversesqrt(m[0][0] * m[0][0] + m[1][0] * m[1][0] + m[2][0] * m[2][0]);

    kQ[0][0]         = m[0][0] * fInvLength;
    kQ[1][0]         = m[1][0] * fInvLength;
    kQ[2][0]         = m[2][0] * fInvLength;

    float fDot       = kQ[0][0] * m[0][1] + kQ[1][0] * m[1][1] + kQ[2][0] * m[2][1];
    kQ[0][1]         = m[0][1] - fDot * kQ[0][0];
    kQ[1][1]         = m[1][1] - fDot * kQ[1][0];
    kQ[2][1]         = m[2][1] - fDot * kQ[2][0];
    fInvLength       = glm::inversesqrt(kQ[0][1] * kQ[0][1] + kQ[1][1] * kQ[1][1] + kQ[2][1] * kQ[2][1]);

    kQ[0][1]         *= fInvLength;
    kQ[1][1]         *= fInvLength;
    kQ[2][1]         *= fInvLength;

    fDot             = kQ[0][0] * m[0][2] + kQ[1][0] * m[1][2] + kQ[2][0] * m[2][2];
    kQ[0][2]         = m[0][2] - fDot * kQ[0][0];
    kQ[1][2]         = m[1][2] - fDot * kQ[1][0];
    kQ[2][2]         = m[2][2] - fDot * kQ[2][0];
    fDot             = kQ[0][1] * m[0][2] + kQ[1][1] * m[1][2] + kQ[2][1] * m[2][2];
    kQ[0][2]         -= fDot * kQ[0][1];
    kQ[1][2]         -= fDot * kQ[1][1];
    kQ[2][2]         -= fDot * kQ[2][1];
    fInvLength       = glm::inversesqrt(kQ[0][2] * kQ[0][2] + kQ[1][2] * kQ[1][2] + kQ[2][2] * kQ[2][2]);

    kQ[0][2]         *= fInvLength;
    kQ[1][2]         *= fInvLength;
    kQ[2][2]         *= fInvLength;

    // guarantee that orthogonal matrix has determinant 1 (no reflections)
    float fDet = kQ[0][0] * kQ[1][1] * kQ[2][2] + kQ[0][1] * kQ[1][2] * kQ[2][0] + kQ[0][2] * kQ[1][0] * kQ[2][1] -
                 kQ[0][2] * kQ[1][1] * kQ[2][0] - kQ[0][1] * kQ[1][0] * kQ[2][2] - kQ[0][0] * kQ[1][2] * kQ[2][1];

    if (fDet < 0.0)
    {
      for (int iRow = 0; iRow < 3; iRow++)
        for (int iCol = 0; iCol < 3; iCol++)
          kQ[iRow][iCol] = -kQ[iRow][iCol];
    }

    // build "right" matrix R
    Mat3 kR;
    kR[0][0]     = kQ[0][0] * m[0][0] + kQ[1][0] * m[1][0] + kQ[2][0] * m[2][0];
    kR[0][1]     = kQ[0][0] * m[0][1] + kQ[1][0] * m[1][1] + kQ[2][0] * m[2][1];
    kR[1][1]     = kQ[0][1] * m[0][1] + kQ[1][1] * m[1][1] + kQ[2][1] * m[2][1];
    kR[0][2]     = kQ[0][0] * m[0][2] + kQ[1][0] * m[1][2] + kQ[2][0] * m[2][2];
    kR[1][2]     = kQ[0][1] * m[0][2] + kQ[1][1] * m[1][2] + kQ[2][1] * m[2][2];
    kR[2][2]     = kQ[0][2] * m[0][2] + kQ[1][2] * m[1][2] + kQ[2][2] * m[2][2];

    kQ           = glm::transpose(kQ); // TK_MOD To column major. Ogre is row major.
    kR           = glm::transpose(kR); // TK_MOD To column major. Ogre is row major.

    // the scaling component
    kD[0]        = kR[0][0];
    kD[1]        = kR[1][1];
    kD[2]        = kR[2][2];

    // the shear component
    float fInvD0 = 1.0f / kD[0];
    kU[0]        = kR[0][1] * fInvD0;
    kU[1]        = kR[0][2] * fInvD0;
    kU[2]        = kR[1][2] / kD[1];
  }

  void ExtractAxes(const Mat4& transform, Vec3& x, Vec3& y, Vec3& z, bool normalize)
  {
    x = glm::column(transform, 0);
    y = glm::column(transform, 1);
    z = glm::column(transform, 2);

    if (normalize)
    {
      x = glm::normalize(x);
      y = glm::normalize(y);
      z = glm::normalize(z);
    }
  }

  /*
   * If the given matrix is projection matrix, then the algorithm gives the
   * clipping planes in view space If the matrix is projection * view,then the
   * algorithm gives the clipping planes in world space If the matrix is
   * projection * view * model, then the algorithm gives the clipping planes in
   * model space
   */
  // http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
  Frustum ExtractFrustum(const Mat4& _projectViewModel, bool normalize)
  {
    Mat4 projectViewModel = glm::transpose(_projectViewModel);

    Frustum frus;

    // Left clipping plane
    frus.planes[4].normal.x = projectViewModel[3][0] + projectViewModel[0][0];
    frus.planes[4].normal.y = projectViewModel[3][1] + projectViewModel[0][1];
    frus.planes[4].normal.z = projectViewModel[3][2] + projectViewModel[0][2];
    frus.planes[4].d        = projectViewModel[3][3] + projectViewModel[0][3];

    // Right clipping plane
    frus.planes[5].normal.x = projectViewModel[3][0] - projectViewModel[0][0];
    frus.planes[5].normal.y = projectViewModel[3][1] - projectViewModel[0][1];
    frus.planes[5].normal.z = projectViewModel[3][2] - projectViewModel[0][2];
    frus.planes[5].d        = projectViewModel[3][3] - projectViewModel[0][3];

    // Top clipping plane
    frus.planes[2].normal.x = projectViewModel[3][0] - projectViewModel[1][0];
    frus.planes[2].normal.y = projectViewModel[3][1] - projectViewModel[1][1];
    frus.planes[2].normal.z = projectViewModel[3][2] - projectViewModel[1][2];
    frus.planes[2].d        = projectViewModel[3][3] - projectViewModel[1][3];

    // Bottom clipping plane
    frus.planes[3].normal.x = projectViewModel[3][0] + projectViewModel[1][0];
    frus.planes[3].normal.y = projectViewModel[3][1] + projectViewModel[1][1];
    frus.planes[3].normal.z = projectViewModel[3][2] + projectViewModel[1][2];
    frus.planes[3].d        = projectViewModel[3][3] + projectViewModel[1][3];

    // Near clipping plane
    frus.planes[0].normal.x = projectViewModel[3][0] + projectViewModel[2][0];
    frus.planes[0].normal.y = projectViewModel[3][1] + projectViewModel[2][1];
    frus.planes[0].normal.z = projectViewModel[3][2] + projectViewModel[2][2];
    frus.planes[0].d        = projectViewModel[3][3] + projectViewModel[2][3];

    // Far clipping plane
    frus.planes[1].normal.x = projectViewModel[3][0] - projectViewModel[2][0];
    frus.planes[1].normal.y = projectViewModel[3][1] - projectViewModel[2][1];
    frus.planes[1].normal.z = projectViewModel[3][2] - projectViewModel[2][2];
    frus.planes[1].d        = projectViewModel[3][3] - projectViewModel[2][3];

    // Normalize the plane equations, if requested
    if (normalize)
    {
      for (int i = 0; i < 6; i++)
      {
        NormalizePlaneEquation(frus.planes[i]);
      }
    }

    return frus;
  }

  float SquareDistancePointToAABB(const Vec3& p, const BoundingBox& b)
  {
    float sqDist = 0.0f;
    for (int i = 0; i < 3; i++)
    {
      // for each axis count any excess distance outside box extents
      float v = p[i];
      if (v < b.min[i])
      {
        sqDist += (b.min[i] - v) * (b.min[i] - v);
      }
      if (v > b.max[i])
      {
        sqDist += (v - b.max[i]) * (v - b.max[i]);
      }
    }
    return sqDist;
  }

  // Returns true if sphere s intersects AABB b, false otherwise
  bool SphereBoxIntersection(const BoundingSphere& s, const BoundingBox& b)
  {
    // Compute squared distance between sphere center and AABB
    // the sqrt(dist) is fine to use as well, but this is faster.
    float sqDist = SquareDistancePointToAABB(s.pos, b);

    // Sphere and AABB intersect if the (squared) distance between them is
    // less than the (squared) sphere radius.
    return sqDist <= s.radius * s.radius;
  }

  bool SpherePointIntersection(const Vec3& spherePos, float sphereRadius, const Vec3& vertex)
  {
    float dist = glm::distance(spherePos, vertex);
    return dist < sphereRadius;
  }

  bool SphereSphereIntersection(const Vec3& spherePos, float sphereRadius, const Vec3& spherePos2, float sphereRadius2)
  {
    float dist = glm::distance(spherePos, spherePos2);
    return dist < (sphereRadius + sphereRadius2);
  }

  bool BoxBoxIntersection(const BoundingBox& box1, const BoundingBox& box2)
  {
    return (box1.min.x <= box2.max.x && box1.max.x >= box2.min.x) &&
           (box1.min.y <= box2.max.y && box1.max.y >= box2.min.y) &&
           (box1.min.z <= box2.max.z && box1.max.z >= box2.min.z);
  }

  bool BoxPointIntersection(const BoundingBox& box, const Vec3& point)
  {
    // Not accept point on bounding box cases.
    if (glm::all(glm::greaterThan(box.max, point)))
    {
      if (glm::all(glm::lessThan(box.min, point)))
      {
        return true;
      }
    }

    return false;
  }

  bool RayBoxIntersection(const Ray& ray, const BoundingBox& box, float& t)
  {
    // r.dir is unit direction vector of ray
    Vec3 invDir = 1.0f / ray.direction;
    Vec3 vmin   = (box.min - ray.position) * invDir;
    Vec3 vmax   = (box.max - ray.position) * invDir;

    float tmin  = glm::compMax(glm::min(vmin, vmax));
    float tmax  = glm::compMin(glm::max(vmin, vmax));

    // if tmax < 0, ray (line) is intersecting AABB,
    // if tmin > tmax, ray doesn't intersect AABB
    // but the whole AABB is behind us
    if (tmax < 0 || tmin > tmax)
    {
      t = tmax;
      return false;
    }
    t = tmin;
    return true;
  }

  bool RectPointIntersection(Vec2 rectMin, Vec2 rectMax, Vec2 point)
  {
    return point.x >= rectMin.x && point.y >= rectMin.y &&
           point.x <= rectMax.x && point.y <= rectMax.y;
  }

  // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
  bool RayTriangleIntersection(const Ray& ray, const Vec3& v0, const Vec3& v1, const Vec3& v2, float& t)
  {
    const float EPSILON = 0.0000001f;
    Vec3 vertex0        = v0;
    Vec3 vertex1        = v1;
    Vec3 vertex2        = v2;
    Vec3 edge1, edge2, h, s, q;
    float a, f, u, v;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    h     = glm::cross(ray.direction, edge2);
    a     = glm::dot(edge1, h);
    if (a > -EPSILON && a < EPSILON)
      return false; // This ray is parallel to this triangle.
    f = 1.0f / a;
    s = ray.position - vertex0;
    u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f)
      return false;
    q = glm::cross(s, edge1);
    v = f * glm::dot(ray.direction, q);
    if (v < 0.0f || (u + v) > 1.0f)
      return false;
    // At this stage we can compute t to find out
    // where the intersection point is on the line.
    t = f * glm::dot(edge2, q);
    if (t > EPSILON) // ray intersection
    {
      return true;
    }
    else
      // This means that there is a line intersection but not a ray intersection
      return false;
  }

  Vec3 CPUSkinning(const SkinVertex* vertex, const Skeleton* skel, DynamicBoneMapPtr dynamicBoneMap)
  {
    Vec3 transformedPos = {};
    for (uint boneIndx = 0; boneIndx < 4; boneIndx++)
    {
      uint currentBone       = (uint) vertex->bones[boneIndx];
      StaticBone* sBone      = skel->m_bones[currentBone];
      Mat4 bindPoseTransform = sBone->m_inverseWorldMatrix;
      ToolKit::Mat4 boneTransform =
          dynamicBoneMap->boneList.find(sBone->m_name)->second.node->GetTransform(TransformationSpace::TS_WORLD);
      transformedPos +=
          Vec3((boneTransform * bindPoseTransform * Vec4(vertex->pos, 1.0f) * vertex->weights[boneIndx]).xyz);
    }
    return transformedPos;
  }

  bool RayMeshIntersection(const Mesh* const mesh, const Ray& ray, float& t, const SkeletonComponent* skelComp)
  {
    float closestPickedDistance = FLT_MAX;
    bool hit                    = false;

#ifndef __EMSCRIPTEN__
    std::mutex updateHit;
    std::for_each(std::execution::par_unseq,
                  mesh->m_faces.begin(),
                  mesh->m_faces.end(),
                  [&updateHit, &t, &closestPickedDistance, &ray, &hit, skelComp, mesh](const Face& face)
                  {
                    Vec3 positions[3] = {face.vertices[0]->pos, face.vertices[1]->pos, face.vertices[2]->pos};
                    if (skelComp != nullptr && mesh->IsSkinned())
                    {
                      SkinMesh* skinMesh = (SkinMesh*) mesh;
                      for (uint32_t vertexIndx = 0; vertexIndx < 3; vertexIndx++)
                      {
                        positions[vertexIndx] = CPUSkinning((SkinVertex*) face.vertices[vertexIndx],
                                                            skinMesh->m_skeleton.get(),
                                                            skelComp->m_map);
                      }
                    }
                    float dist = FLT_MAX;
                    if (RayTriangleIntersection(ray, positions[0], positions[1], positions[2], dist))
                    {
                      std::lock_guard<std::mutex> guard(updateHit);
                      if (dist < closestPickedDistance && t >= 0.0f)
                      {
                        t                     = dist;
                        closestPickedDistance = dist;
                        hit                   = true;
                      }
                    }
                  });
#else
    for (const Face& face : mesh->m_faces)
    {
      float dist = FLT_MAX;
      if (RayTriangleIntersection(ray, face.vertices[0]->pos, face.vertices[1]->pos, face.vertices[2]->pos, dist))
      {
        if (dist < closestPickedDistance && t >= 0.0f)
        {
          t                     = dist;
          closestPickedDistance = dist;
          hit                   = true;
        }
      }
    }
#endif

    return hit;
  }

  TK_API uint FindMeshIntersection(const class Entity* const ntt, const Ray& rayInWorldSpace, float& t)
  {
    SkeletonComponent* skel = ntt->GetComponent<SkeletonComponent>().get();

    MeshComponentPtrArray meshComps;
    ntt->GetComponent<MeshComponent>(meshComps);

    MeshRawCPtrArray meshes;
    for (MeshComponentPtr meshComp : meshComps)
    {
      meshComp->GetMeshVal()->GetAllMeshes(meshes);
    }

    struct meshTrace
    {
      float dist;
      uint indx;
    };

    std::vector<meshTrace> meshTraces;
    for (uint i = 0; i < meshes.size(); i++)
    {
      // There is a special case for SkinMeshes, because
      // m_clientSideVertices.size() here always accesses to Mesh's vertex
      // array (Vertex*) but it should've access to SkinMesh's vertex
      // array (SkinVertex*). That's why SkinMeshes checked with a cast
      const Mesh* const mesh = meshes[i];
      if (mesh->IsSkinned())
      {
        SkinMesh* skinMesh = (SkinMesh*) mesh;
        if (skinMesh->m_clientSideVertices.size() && skel)
        {
          meshTraces.push_back({TK_FLT_MAX, i});
        }
      }
      else if (mesh->m_clientSideVertices.size())
      {
        meshTraces.push_back({TK_FLT_MAX, i});
      }
    }

    if (meshTraces.size() == 0)
    {
      t = 0.0f;
      return TK_UINT_MAX;
    }

    Ray rayInObjectSpace       = rayInWorldSpace;
    Mat4 ts                    = ntt->m_node->GetTransform(TransformationSpace::TS_WORLD);
    Mat4 its                   = glm::inverse(ts);
    rayInObjectSpace.position  = its * Vec4(rayInWorldSpace.position, 1.0f);
    rayInObjectSpace.direction = its * Vec4(rayInWorldSpace.direction, 0.0f);

#ifndef __EMSCRIPTEN__
    std::for_each(std::execution::par_unseq,
                  meshTraces.begin(),
                  meshTraces.end(),
                  [rayInObjectSpace, skel, &meshes](meshTrace& trace)
                  {
                    float t = TK_FLT_MAX;

                    if (RayMeshIntersection(meshes[trace.indx], rayInObjectSpace, t, skel))
                    {
                      trace.dist = t;
                    }
                  });
#else
    for (meshTrace& trace : meshTraces)
    {
      float t = FLT_MAX;

      if (RayMeshIntersection(meshes[trace.indx], rayInObjectSpace, t, skel))
      {
        trace.dist = t;
      }
    }
#endif

    t                = TK_FLT_MAX;
    uint closestIndx = TK_UINT_MAX;
    for (const meshTrace& trace : meshTraces)
    {
      if (trace.dist < t)
      {
        t           = trace.dist;
        closestIndx = trace.indx;
      }
    }
    return closestIndx;
  }

  /*
   * When the plane equation is not normalized, the distance of a point to the
   * plane: If distance < 0 , then the point p lies in the negative halfspace.
   * If distance = 0 , then the point p lies in the plane.
   * If distance > 0 , then the point p lies in the positive halfspace.
   */
  IntersectResult FrustumBoxIntersection(const Frustum& frustum, const BoundingBox& box)
  {
    Vec3 vmin, vmax;
    IntersectResult res = IntersectResult::Inside;

    for (int i = 0; i < 6; ++i)
    {
      // X axis.
      if (frustum.planes[i].normal.x > 0)
      {
        vmin.x = box.max.x;
        vmax.x = box.min.x;
      }
      else
      {
        vmin.x = box.min.x;
        vmax.x = box.max.x;
      }

      // Y axis.
      if (frustum.planes[i].normal.y > 0)
      {
        vmin.y = box.max.y;
        vmax.y = box.min.y;
      }
      else
      {
        vmin.y = box.min.y;
        vmax.y = box.max.y;
      }

      // Z axis.
      if (frustum.planes[i].normal.z > 0)
      {
        vmin.z = box.max.z;
        vmax.z = box.min.z;
      }
      else
      {
        vmin.z = box.min.z;
        vmax.z = box.max.z;
      }

      float distmin = glm::dot(frustum.planes[i].normal, vmin) + frustum.planes[i].d;
      if (distmin > 0)
      {
        float distmax = glm::dot(frustum.planes[i].normal, vmax) + frustum.planes[i].d;
        if (distmax <= 0)
        {
          res = IntersectResult::Intersect;
        }
        else
        {
          // Inside
        }
      }
      else if (distmin < 0)
      {
        return IntersectResult::Outside;
      }
      else
      {
        res = IntersectResult::Intersect;
      }
    }

    return res;
  }

  Quaternion QuaternionLookAt(Vec3 direction)
  {
    Mat3 Result {};
    Result[2] = -glm::normalize(direction);
    Result[0] = glm::normalize(glm::cross(Y_AXIS, Result[2]));
    Result[1] = glm::cross(Result[2], Result[0]);
    return glm::quat_cast(Result);
  }
  
  // frustum should be normalized
  bool FrustumSphereIntersection(const Frustum& frustum, const Vec3& pos, float radius)
  {
    // check each frustum plane against sphere
    for (int i = 0; i < 6; i++)
    {
      const PlaneEquation& plane = frustum.planes[i];
      float signedDistance       = glm::dot(plane.normal, pos) + plane.d;
      if (signedDistance < -radius)
      {
        return false; // Sphere is fully outside this plane, no intersection
      }
    }
    return true;
  }

  bool ConePointIntersection(Vec3 conePos, Vec3 coneDir, float coneHeight, float coneAngle, Vec3 point)
  {
    Vec3 pointToCone = point - conePos;
    return glm::dot(glm::normalize(pointToCone), coneDir) > 1.0f - (glm::radians(coneAngle) / glm::pi<float>()) &&
           glm::length(pointToCone) < coneHeight;
  }

  bool FrustumConeIntersect(const Frustum& frustum,
                            Vec3 conePos, 
                            Vec3 coneDir, 
                            float coneHeight, 
                            float coneAngle)
  {
    return true;
    const int numSteps = 3;
    float outerCircleRadius = coneHeight * glm::tan(glm::radians(coneAngle * 0.5f));
    
    for (int i = 1; i <= numSteps; ++i) 
    {
      const PlaneEquation& plane = frustum.planes[i];
      Vec3 point                 = conePos + (coneDir * (coneHeight / i));
      if (FrustumSphereIntersection(frustum, point, outerCircleRadius / i))
      {
        return true;
      }
    }
    return false; // Intersection
  }

  bool RayPlaneIntersection(const Ray& ray, const PlaneEquation& plane, float& t)
  {
    float denom = glm::dot(ray.direction, plane.normal);
    // Ray and plane facing(-). Not parallel (0) or ray facing plane's back (+).
    if (glm::lessThan(denom, 0.0f))
    {
      t = -(glm::dot(plane.normal, ray.position) - plane.d) / denom;
      // On (0) or in front of (+) the plane.
      return glm::greaterThanEqual(t, 0.0f);
    }

    return false;
  }

  // https://gist.github.com/wwwtyro/beecc31d65d1004f5a9d
  bool RaySphereIntersection(const Ray& ray, const BoundingSphere& sphere, float& t)
  {
    Vec3 r0    = ray.position;
    Vec3 rd    = ray.direction;
    Vec3 s0    = sphere.pos;
    float sr   = sphere.radius;

    float a    = glm::dot(rd, rd);
    Vec3 s0_r0 = r0 - s0;
    float b    = 2.0f * glm::dot(rd, s0_r0);
    float c    = glm::dot(s0_r0, s0_r0) - (sr * sr);
    if (b * b - 4.0f * a * c < 0.0f)
    {
      return false;
    }

    t = (-b - glm::sqrt((b * b) - 4.0f * a * c)) / (2.0f * a);
    return true;
  }

  bool LinePlaneIntersection(const Ray& ray, const PlaneEquation& plane, float& t)
  {
    float denom = glm::dot(ray.direction, plane.normal);
    if (glm::notEqual(denom, 0.0f)) // Not parallel (0).
    {
      t = -(glm::dot(plane.normal, ray.position) - plane.d) / denom;
      return true;
    }

    return false;
  }

  Vec3 PointOnRay(const Ray& ray, float t) { return ray.position + ray.direction * t; }

  // https://forum.unity.com/threads/how-do-i-find-the-closest-point-on-a-line.340058/
  TK_API Vec3 ProjectPointOntoLine(const Ray& baseLine, const Vec3& point)
  {
    Vec3 v  = point - baseLine.position;
    float d = dot(v, baseLine.direction);
    return baseLine.position + (baseLine.direction * d);
  }

  // http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
  void NormalizePlaneEquation(PlaneEquation& plane)
  {
    float mag      = glm::length(plane.normal);
    plane.normal.x = plane.normal.x / mag;
    plane.normal.y = plane.normal.y / mag;
    plane.normal.z = plane.normal.z / mag;
    plane.d        = plane.d / mag;
  }

  bool FrustumTest(const Frustum& frustum, const BoundingBox& box)
  {
    IntersectResult res = FrustumBoxIntersection(frustum, box);
    if (res == IntersectResult::Outside)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  void FrustumCull(EntityRawPtrArray& entities, Camera* camera)
  {
    // Frustum cull
    Mat4 pr         = camera->GetProjectionMatrix();
    Mat4 v          = camera->GetViewMatrix();
    Frustum frustum = ExtractFrustum(pr * v, false);

    auto delFn      = [frustum](Entity* ntt) -> bool { return FrustumTest(frustum, ntt->GetAABB(true)); };
    erase_if(entities, delFn);
  }

  void FrustumCull(RenderJobArray& jobs, Camera* camera)
  {
    // Frustum cull
    Mat4 pr         = camera->GetProjectionMatrix();
    Mat4 v          = camera->GetViewMatrix();
    Frustum frustum = ExtractFrustum(pr * v, false);

    auto delFn      = [frustum](RenderJob& job) -> bool { return FrustumTest(frustum, job.BoundingBox); };
    erase_if(jobs, delFn);
  }

  void TransformAABB(BoundingBox& box, const Mat4& transform)
  {
    Vec3Array corners;
    GetCorners(box, corners);

    // Transform and update aabb.
    box = BoundingBox();
    for (int i = 0; i < 8; i++)
    {
      Vec3 v  = transform * Vec4(corners[i], 1.0f);
      box.min = glm::min(v, box.min);
      box.max = glm::max(v, box.max);
    }
  }

  void GetCorners(const BoundingBox& box, Vec3Array& corners)
  {
    // Calculate all 8 edges.
    Vec3 maxtr = box.max;

    Vec3 maxtl = box.max;
    maxtl.x    = box.min.x;

    Vec3 maxbr = maxtr;
    maxbr.y    = box.min.y;

    Vec3 maxbl = maxtl;
    maxbl.y    = box.min.y;

    Vec3 minbl = maxbl;
    minbl.z    = box.min.z;

    Vec3 minbr = maxbr;
    minbr.z    = box.min.z;

    Vec3 mintl = maxtl;
    mintl.z    = box.min.z;

    Vec3 mintr = maxtr;
    mintr.z    = box.min.z;

    corners    = {mintl, mintr, minbr, minbl, maxtl, maxtr, maxbr, maxbl};
  }

  PlaneEquation PlaneFrom(const Vec3 pnts[3])
  {
    // Expecting 3 non coplanar points in CCW order.
    Vec3 v1 = pnts[1] - pnts[0];
    Vec3 v2 = pnts[2] - pnts[0];

    PlaneEquation eq;
    eq.normal = glm::normalize(glm::cross(v1, v2));
    eq.d      = -glm::dot(eq.normal, pnts[0]);

    return eq;
  }

  PlaneEquation PlaneFrom(Vec3 point, Vec3 normal)
  {
    assert(glm::isNormalized(normal, 0.0001f) && "Normalized vector expected.");

    PlaneEquation plane = {normal, 0.0f};
    plane.d             = glm::dot(point, normal) / glm::dot(normal, normal);

    return plane;
  }

  float SignedDistance(const PlaneEquation& plane, const Vec3& pnt)
  {
    assert(glm::isNormalized(plane.normal, 0.0001f) && "Normalized vector expected.");

    Vec3 planeOrig = plane.normal * plane.d;
    Vec3 checkPnt  = pnt - planeOrig;

    return glm::dot(checkPnt, plane.normal);
  }

  Vec3 ProjectPointOntoPlane(const PlaneEquation& plane, const Vec3& pnt)
  {
    assert(glm::isNormalized(plane.normal, 0.0001f) && "Normalized vector expected.");

    return pnt - glm::dot(plane.normal, pnt) * plane.normal;
  }

  Vec3 Interpolate(const Vec3& vec1, const Vec3& vec2, float ratio) { return (vec2 - vec1) * ratio + vec1; }

  void ToSpherical(Vec3 p, float& r, float& zenith, float& azimuth)
  {
    r       = glm::length(p);
    azimuth = glm::atan(p.x, p.z);
    zenith  = glm::acos(p.y / r);
  }

  Vec3 ToCartesian(float r, float zenith, float azimuth)
  {
    return Vec3(r * glm::sin(zenith) * glm::sin(azimuth),
                r * glm::cos(zenith),
                r * glm::sin(zenith) * glm::cos(azimuth));
  }

  Quaternion RotationTo(Vec3 a, Vec3 b)
  {
    a = glm::normalize(a);
    b = glm::normalize(b);

    Vec3 axis;
    float d = glm::dot(a, b);
    if (glm::equal<float>(d, 1.0f))
    {
      return Quaternion(); // Vectors are colinear.
    }
    else if (glm::equal<float>(d, -1.0f)) // Vectors are oposite, align them.
    {
      axis = Orthogonal(a);
    }
    else
    {
      axis = glm::normalize(glm::cross(a, b));
    }

    float rad = glm::acos(d);
    return glm::angleAxis(rad, axis);
  }

  // Converted from OgreVector3.h perpendicular()
  TK_API Vec3 Orthogonal(const Vec3& v)
  {
    static const float fSquareZero = static_cast<float>(1e-06 * 1e-06);
    Vec3 perp                      = glm::cross(v, X_AXIS);
    // Check length
    if (glm::length2(perp) < fSquareZero)
    {
      /* This vector is the Y axis multiplied by a scalar, so we have
      to use another axis.
      */
      perp = glm::cross(v, Y_AXIS);
    }

    return glm::normalize(perp);
  }

  bool PointInsideBBox(const Vec3& point, const Vec3& max, const Vec3& min)
  {
    return (point.x <= max.x && point.x >= min.x && point.y <= max.y && point.y >= min.y && point.z <= max.z &&
            point.z >= min.z);
  }

  Vec3Array GenerateRandomSamplesOnHemisphere(int numSamples, float bias)
  {
    Vec3Array points;
    points.reserve(numSamples);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0, 1.0);

    for (int i = 0; i < numSamples; i++)
    {
      float theta = 2 * glm::pi<float>() * dis(gen);
      float phi   = std::acos(1 - bias * dis(gen));

      float x     = std::sin(phi) * std::cos(theta);
      float y     = std::sin(phi) * std::sin(theta);
      float z     = std::cos(phi);

      points.emplace_back(x, y, z);
    }

    return points;
  }

  Vec3Array GenerateRandomSamplesInHemisphere(int numSamples, float bias)
  {
    Vec3Array samples;

    // Generate random samples on the hemisphere with random length between 0
    // and 1
    for (int i = 0; i < numSamples; ++i)
    {
      float theta  = glm::linearRand(0.f, 2.f * glm::pi<float>());

      // Calculate phi based on the parameter
      float phi    = glm::acos(1.f - bias * glm::linearRand(0.f, 1.f));

      float length = glm::linearRand(0.f, 1.f);
      float x      = glm::sin(phi) * glm::cos(theta) * length;
      float y      = glm::sin(phi) * glm::sin(theta) * length;
      float z      = glm::cos(phi) * length;
      samples.push_back(glm::vec3(x, y, z));
    }

    return samples;
  }

} // namespace ToolKit
