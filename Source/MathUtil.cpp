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

// https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
bool ToolKit::RayBoxIntersection(const Ray& ray, const BoundingBox& box, float& t)
{
	// r.dir is unit direction vector of ray
	glm::vec3 dirfrac;
	dirfrac.x = 1.0f / ray.direction.x;
	dirfrac.y = 1.0f / ray.direction.y;
	dirfrac.z = 1.0f / ray.direction.z;

	// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
	// r.org is origin of ray
	float t1 = (box.min.x - ray.position.x) * dirfrac.x;
	float t2 = (box.max.x - ray.position.x) * dirfrac.x;
	float t3 = (box.min.y - ray.position.y) * dirfrac.y;
	float t4 = (box.max.y - ray.position.y) * dirfrac.y;
	float t5 = (box.min.z - ray.position.z) * dirfrac.z;
	float t6 = (box.max.z - ray.position.z) * dirfrac.z;

	float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
	float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

	// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
	if (tmax < 0)
	{
		t = tmax;
		return false;
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
	{
		t = tmax;
		return false;
	}

	t = tmin;
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
		glm::vec3 triangle[3];
		for (uint i = 0; i < mesh->m_clientSideVertices.size(); i += 3)
		{
			if (mesh->m_clientSideIndices.empty())
			{
				for (int j = i; j < 3; j++)
				{
					triangle[j] = mesh->m_clientSideVertices[j].pos;
				}
			}
			else
			{
				for (int j = i; j < 3; j++)
				{
					int indx = mesh->m_clientSideIndices[j];
					triangle[j] = mesh->m_clientSideVertices[indx].pos;
				}
			}

			if (RayTriangleIntersection(ray, triangle[0], triangle[1], triangle[2], t))
			{
				return true;
			}
		}
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
