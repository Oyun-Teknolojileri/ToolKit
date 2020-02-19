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

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution
bool ToolKit::RayTriangleIntersection(const Ray& ray, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t)
{
	// compute plane's normal
	glm::vec3 v0v1 = v1 - v0;
	glm::vec3 v0v2 = v2 - v0;
	// no need to normalize
	glm::vec3 N = glm::cross(v0v1, v0v2); // N 
	float area2 = (float)N.length();

	// Step 1: finding P

	// check if ray and plane are parallel ?
	float NdotRayDirection = glm::dot(N, ray.direction);
	if (glm::abs(NdotRayDirection) < glm::epsilon<float>()) // almost 0 
		return false; // they are parallel so they don't intersect ! 

// compute d parameter using equation 2
	float d = glm::dot(N, v0);

	// compute t (equation 3)
	t = (glm::dot(N, ray.position) + d) / NdotRayDirection;
	// check if the triangle is in behind the ray
	if (t < 0) return false; // the triangle is behind 

	// compute the intersection point using equation 1
	glm::vec3 P = ray.position + t * ray.direction;

	// Step 2: inside-outside test
	glm::vec3 C; // vector perpendicular to triangle's plane 

	// edge 0
	glm::vec3 edge0 = v1 - v0;
	glm::vec3 vp0 = P - v0;
	C = glm::cross(edge0, vp0);
	if (glm::dot(N, C) < 0) return false; // P is on the right side 

	// edge 1
	glm::vec3 edge1 = v2 - v1;
	glm::vec3 vp1 = P - v1;
	C = glm::cross(edge1, vp1);
	if (glm::dot(N, C) < 0)  return false; // P is on the right side 

	// edge 2
	glm::vec3 edge2 = v0 - v2;
	glm::vec3 vp2 = P - v2;
	C = glm::cross(edge2, vp2);
	if (glm::dot(N, C) < 0) return false; // P is on the right side; 

	return true; // this ray hits the triangle 
}

bool ToolKit::RayMeshIntersection(Mesh* const mesh, const Ray& ray, float& t)
{
	std::vector<Mesh*> meshes;
	mesh->GetAllMeshes(meshes);
	for (Mesh* const mesh : meshes)
	{

	}

	return false;
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

glm::quat ToolKit::RotationTo(glm::vec3 a, glm::vec3 b)
{
	a = glm::normalize(a);
	b = glm::normalize(b);
	float rad = glm::acos(glm::dot(a, b));
	glm::vec3 axis = glm::normalize(glm::cross(a, b));

	return glm::angleAxis(rad, axis);
}
