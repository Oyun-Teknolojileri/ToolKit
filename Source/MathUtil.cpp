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

// http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
ToolKit::Frustum ToolKit::ExtractFrustum(const glm::mat4& modelViewProject)
{
	Frustum frustum;

	// Left clipping plane
	frustum.planes[0].normal.x = modelViewProject[3][0] + modelViewProject[0][0];
	frustum.planes[0].normal.y = modelViewProject[3][1] + modelViewProject[0][1];
	frustum.planes[0].normal.z = modelViewProject[3][2] + modelViewProject[0][2];
	frustum.planes[0].d = modelViewProject[3][3] + modelViewProject[0][3];

	// Right clipping plane
	frustum.planes[1].normal.x = modelViewProject[3][0] - modelViewProject[0][0];
	frustum.planes[1].normal.y = modelViewProject[3][1] - modelViewProject[0][1];
	frustum.planes[1].normal.z = modelViewProject[3][2] - modelViewProject[0][2];
	frustum.planes[1].d = modelViewProject[3][3] - modelViewProject[0][3];

	// Top clipping plane
	frustum.planes[2].normal.x = modelViewProject[3][0] + modelViewProject[1][0];
	frustum.planes[2].normal.y = modelViewProject[3][1] + modelViewProject[1][1];
	frustum.planes[2].normal.z = modelViewProject[3][2] + modelViewProject[1][2];
	frustum.planes[2].d = modelViewProject[3][3] + modelViewProject[1][3];

	// Bottom clipping plane
	frustum.planes[3].normal.x = modelViewProject[3][0] - modelViewProject[1][0];
	frustum.planes[3].normal.y = modelViewProject[3][1] - modelViewProject[1][1];
	frustum.planes[3].normal.z = modelViewProject[3][2] - modelViewProject[1][2];
	frustum.planes[3].d = modelViewProject[3][3] - modelViewProject[1][3];

	// Near clipping plane
	frustum.planes[4].normal.x = modelViewProject[3][0] + modelViewProject[2][0];
	frustum.planes[4].normal.y = modelViewProject[3][1] + modelViewProject[2][1];
	frustum.planes[4].normal.z = modelViewProject[3][2] + modelViewProject[2][2];
	frustum.planes[4].d = modelViewProject[3][3] + modelViewProject[2][3];

	// Far clipping plane
	frustum.planes[5].normal.x = modelViewProject[3][0] - modelViewProject[2][0];
	frustum.planes[5].normal.y = modelViewProject[3][1] - modelViewProject[2][1];
	frustum.planes[5].normal.z = modelViewProject[3][2] - modelViewProject[2][2];
	frustum.planes[5].d = modelViewProject[3][3] - modelViewProject[2][3];
	
	// Normalize the plane equations, if requested
	for (int i = 0; i < 6; i++)
	{
		NormalzePlaneEquation(frustum.planes[i]);
	}

	return frustum;
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

// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
bool ToolKit::RayTriangleIntersection(const Ray& ray, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t)
{
	const float EPSILON = 0.0000001f;
	glm::vec3 vertex0 = v0;
	glm::vec3 vertex1 = v1;
	glm::vec3 vertex2 = v2;
	glm::vec3 edge1, edge2, h, s, q;
	float a, f, u, v;
	edge1 = vertex1 - vertex0;
	edge2 = vertex2 - vertex0;
	h = glm::cross(ray.direction, edge2);
	a = glm::dot(edge1, h);
	if (a > -EPSILON && a < EPSILON)
		return false;    // This ray is parallel to this triangle.
	f = 1.0f / a;
	s = ray.position - vertex0;
	u = f * glm::dot(s, h);
	if (u < 0.0 || u > 1.0)
		return false;
	q = glm::cross(s, edge1);
	v = f * glm::dot(ray.direction, q);
	if (v < 0.0 || u + v > 1.0)
		return false;
	// At this stage we can compute t to find out where the intersection point is on the line.
	t = f * glm::dot(edge2, q);
	if (t > EPSILON) // ray intersection
	{
		return true;
	}
	else // This means that there is a line intersection but not a ray intersection.
		return false;
}

bool ToolKit::RayMeshIntersection(Mesh* const mesh, const Ray& ray, float& t)
{
	std::vector<Mesh*> meshes;
	mesh->GetAllMeshes(meshes);
	float closestPickedDistance = FLT_MAX;
	bool hit = false;

	for (Mesh* const mesh : meshes)
	{
		glm::vec3 triangle[3];
		size_t triCnt = mesh->m_clientSideIndices.size() / 3;
		for (size_t i = 0; i < triCnt; i++)
		{
			if (mesh->m_clientSideIndices.empty())
			{
				for (size_t j = 0; j < 3; j++)
				{
					triangle[j] = mesh->m_clientSideVertices[i * 3 + j].pos;
				}
			}
			else
			{
				for (size_t j = 0; j < 3; j++)
				{
					size_t indx = mesh->m_clientSideIndices[i * 3 + j];
					triangle[j] = mesh->m_clientSideVertices[indx].pos;
				}
			}

			float dist = FLT_MAX;
			if (RayTriangleIntersection(ray, triangle[0], triangle[1], triangle[2], dist))
			{
				if (dist < closestPickedDistance && t > 0.0f)
				{
					t = dist;
					closestPickedDistance = dist;
					hit = true;
				}
			}
		}
	}

	return hit;
}

// https://old.cescg.org/CESCG-2002/DSykoraJJelinek/
int ToolKit::FrustumBoxIntersection(const Frustum& frustum, const BoundingBox& box)
{
	float m, n; int i, result = 1;
	glm::vec3 b[2] = { box.min, box.max };

	for (i = 0; i < 6; i++) 
	{
		const PlaneEquation* p = &frustum.planes[i];

		m = (p->normal.x * b[(int)p->normal.x].x) + (p->normal.y * b[(int)p->normal.y].y) + (p->normal.z * b[(int)p->normal.z].z);
		if (m > -p->d)
		{
			return 0;
		}
		
		n = (p->normal.x * b[(int)p->normal.x].x) + (p->normal.y * b[(int)p->normal.y].y) + (p->normal.z * b[(int)p->normal.z].z);
		if (n > -p->d)
		{
			result = 2;
		}
	} 
	
	return result;
}

// http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
void ToolKit::NormalzePlaneEquation(PlaneEquation& plane)
{
	float mag = glm::length(plane.normal);
	plane.normal.x = plane.normal.x / mag;
	plane.normal.y = plane.normal.y / mag;
	plane.normal.z = plane.normal.z / mag;
	plane.d = plane.d / mag;
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
