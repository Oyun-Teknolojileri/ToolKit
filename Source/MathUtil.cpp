#include "stdafx.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "DebugNew.h"

void DecomposeMatrix(const glm::mat4& transform, glm::vec3& position, glm::quat& rotation, glm::vec3& scale)
{
  position = glm::column(transform, 3).xyz;

  glm::vec3 col0 = glm::column(transform, 0).xyz;
  float sx = glm::length(col0);
  col0 /= sx;

  glm::vec3 col1 = glm::column(transform, 1).xyz;
  float sy = glm::length(col1);
  col1 /= sy;

  glm::vec3 col2 = glm::column(transform, 2).xyz;
  float sz = glm::length(col2);
  col2 /= sz;

  glm::mat3 rotm;
  rotm = glm::column(rotm, 0, col0);
  rotm = glm::column(rotm, 1, col1);
  rotm = glm::column(rotm, 2, col2);

  rotation = glm::toQuat(rotm);
  scale = glm::vec3(sx, sy, sz);
}

void DecomposeMatrix(const glm::mat4& transform, glm::vec3& position, glm::quat& rotation)
{
  glm::vec3 tmp;
  DecomposeMatrix(transform, position, rotation, tmp);
}

bool SpherePointIntersection(const glm::vec3& spherePos, float sphereRadius, const glm::vec3& vertex)
{
  float dist = glm::distance(spherePos, vertex);
  return dist < sphereRadius;
}

bool SphereSphereIntersection(const glm::vec3& spherePos, float sphereRadius, const glm::vec3& spherePos2, float sphereRadius2)
{
  float dist = glm::distance(spherePos, spherePos2);
  return dist < (sphereRadius + sphereRadius2);
}

glm::vec3 Interpolate(const glm::vec3& vec1, const glm::vec3& vec2, float ratio)
{
  return (vec2 - vec1) * ratio + vec1;
}

void ToSpherical(glm::vec3 p, float& r, float& zenith, float& azimuth)
{
  r = glm::length(p);
  azimuth = glm::atan(p.x, p.z);
  zenith = glm::acos(p.y / r);
}

glm::vec3 ToCartesian(float r, float zenith, float azimuth)
{
  return glm::vec3(r * glm::sin(zenith) * glm::sin(azimuth),  r * glm::cos(zenith), r * glm::sin(zenith) * glm::cos(azimuth));
}
