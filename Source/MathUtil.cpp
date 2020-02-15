#include "stdafx.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "DebugNew.h"

void ToolKit::DecomposeMatrix(const glm::mat4& transform, glm::vec3& position, glm::quat& rotation, glm::vec3& scale)
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

void ToolKit::DecomposeMatrix(const glm::mat4& transform, glm::vec3& position, glm::quat& rotation)
{
	glm::vec3 tmp;
	DecomposeMatrix(transform, position, rotation, tmp);
}

bool ToolKit::SpherePointIntersection(const glm::vec3& spherePos, float sphereRadius, const glm::vec3& vertex)
{
	float dist = glm::distance(spherePos, vertex);
	return dist < sphereRadius;
}

bool ToolKit::SphereSphereIntersection(const glm::vec3& spherePos, float sphereRadius, const glm::vec3& spherePos2, float sphereRadius2)
{
	float dist = glm::distance(spherePos, spherePos2);
	return dist < (sphereRadius + sphereRadius2);
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
bool ToolKit::RayBoxIntersection(const Ray& ray, const BoundingBox& box)
{
  float tmin = (box.min.x - ray.position.x) / ray.direction.x;
  float tmax = (box.max.x - ray.position.x) / ray.direction.x;

  if (tmin > tmax) std::swap(tmin, tmax);

  float tymin = (box.min.y - ray.position.y) / ray.direction.y;
  float tymax = (box.max.y - ray.position.y) / ray.direction.y;

  if (tymin > tymax) std::swap(tymin, tymax);

  if ((tmin > tymax) || (tymin > tmax))
    return false;

  if (tymin > tmin)
    tmin = tymin;

  if (tymax < tmax)
    tmax = tymax;

  float tzmin = (box.min.z - ray.position.z) / ray.direction.z;
  float tzmax = (box.max.z - ray.position.z) / ray.direction.z;

  if (tzmin > tzmax) std::swap(tzmin, tzmax);

  if ((tmin > tzmax) || (tzmin > tmax))
    return false;

  if (tzmin > tmin)
    tmin = tzmin;

  if (tzmax < tmax)
    tmax = tzmax;

  return true;
}

glm::vec3 ToolKit::Interpolate(const glm::vec3& vec1, const glm::vec3& vec2, float ratio)
{
	return (vec2 - vec1) * ratio + vec1;
}

void ToolKit::ToSpherical(glm::vec3 p, float& r, float& zenith, float& azimuth)
{
	r = glm::length(p);
	azimuth = glm::atan(p.x, p.z);
	zenith = glm::acos(p.y / r);
}

glm::vec3 ToolKit::ToCartesian(float r, float zenith, float azimuth)
{
	return glm::vec3(r * glm::sin(zenith) * glm::sin(azimuth), r * glm::cos(zenith), r * glm::sin(zenith) * glm::cos(azimuth));
}
