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

void ToolKit::ExtractAxes(const glm::mat4& transform, glm::vec3& x, glm::vec3& y, glm::vec3& z, bool normalize)
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

// http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
ToolKit::Frustum ToolKit::ExtractFrustum(const glm::mat4& projectViewModel)
{
	Frustum frustum;

	// Left clipping plane
	frustum.planes[0].normal.x = projectViewModel[3][0] + projectViewModel[0][0];
	frustum.planes[0].normal.y = projectViewModel[3][1] + projectViewModel[0][1];
	frustum.planes[0].normal.z = projectViewModel[3][2] + projectViewModel[0][2];
	frustum.planes[0].d = projectViewModel[3][3] + projectViewModel[0][3];

	// Right clipping plane
	frustum.planes[1].normal.x = projectViewModel[3][0] - projectViewModel[0][0];
	frustum.planes[1].normal.y = projectViewModel[3][1] - projectViewModel[0][1];
	frustum.planes[1].normal.z = projectViewModel[3][2] - projectViewModel[0][2];
	frustum.planes[1].d = projectViewModel[3][3] - projectViewModel[0][3];

	// Top clipping plane
	frustum.planes[2].normal.x = projectViewModel[3][0] + projectViewModel[1][0];
	frustum.planes[2].normal.y = projectViewModel[3][1] + projectViewModel[1][1];
	frustum.planes[2].normal.z = projectViewModel[3][2] + projectViewModel[1][2];
	frustum.planes[2].d = projectViewModel[3][3] + projectViewModel[1][3];

	// Bottom clipping plane
	frustum.planes[3].normal.x = projectViewModel[3][0] - projectViewModel[1][0];
	frustum.planes[3].normal.y = projectViewModel[3][1] - projectViewModel[1][1];
	frustum.planes[3].normal.z = projectViewModel[3][2] - projectViewModel[1][2];
	frustum.planes[3].d = projectViewModel[3][3] - projectViewModel[1][3];

	// Near clipping plane
	frustum.planes[4].normal.x = projectViewModel[3][0] + projectViewModel[2][0];
	frustum.planes[4].normal.y = projectViewModel[3][1] + projectViewModel[2][1];
	frustum.planes[4].normal.z = projectViewModel[3][2] + projectViewModel[2][2];
	frustum.planes[4].d = projectViewModel[3][3] + projectViewModel[2][3];

	// Far clipping plane
	frustum.planes[5].normal.x = projectViewModel[3][0] - projectViewModel[2][0];
	frustum.planes[5].normal.y = projectViewModel[3][1] - projectViewModel[2][1];
	frustum.planes[5].normal.z = projectViewModel[3][2] - projectViewModel[2][2];
	frustum.planes[5].d = projectViewModel[3][3] - projectViewModel[2][3];
	
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
	if (u < 0.0f || u > 1.0f)
		return false;
	q = glm::cross(s, edge1);
	v = f * glm::dot(ray.direction, q);
	if (v < 0.0f || (u + v) > 1.0f)
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

// https://gist.github.com/Kinwailo/d9a07f98d8511206182e50acda4fbc9b
ToolKit::IntersectResult ToolKit::FrustumBoxIntersection(const Frustum& frustum, const BoundingBox& box)
{
	glm::vec3 vmin, vmax;
	IntersectResult res = IntersectResult::Inside;

	for (int i = 0; i < 6; ++i)
	{
		// X axis.
		if (frustum.planes[i].normal.x > 0)
		{
			vmin.x = box.min.x;
			vmax.x = box.max.x;
		}
		else
		{
			vmin.x = box.max.x;
			vmax.x = box.min.x;
		}

		// Y axis.
		if (frustum.planes[i].normal.y > 0)
		{
			vmin.y = box.min.y;
			vmax.y = box.max.y;
		}
		else 
		{
			vmin.y = box.max.y;
			vmax.y = box.min.y;
		}

		// Z axis.
		if (frustum.planes[i].normal.z > 0)
		{
			vmin.z = box.min.z;
			vmax.z = box.max.z;
		}
		else
		{
			vmin.z = box.max.z;
			vmax.z = box.min.z;
		}

		if (glm::dot(frustum.planes[i].normal, vmin) + frustum.planes[i].d > 0)
		{
			return IntersectResult::Outside;
		}

		if (glm::dot(frustum.planes[i].normal, vmax) + frustum.planes[i].d >= 0)
		{
			res = IntersectResult::Intersect;
		}
	}

	return res;
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection
bool ToolKit::RayPlaneIntersection(const Ray& ray, const PlaneEquation& plane, float& t)
{
	// assuming vectors are all normalized
	float denom = glm::dot(plane.normal, ray.direction);
	if (denom < 0.0001f) // Scratch pixel made a mistake here.
	{
		glm::vec3 p0 = plane.normal * plane.d;
		glm::vec3 p0l0 = p0 - ray.position;
		t = glm::dot(p0l0, plane.normal) / denom;
		return (t >= 0.0f);
	}

	return false;
}

glm::vec3 ToolKit::PointOnRay(const Ray& ray, float t)
{
	return ray.position + ray.direction * t;
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

void ToolKit::TransformAABB(BoundingBox& box, const glm::mat4& transform)
{
	// Calculate all 8 edges.
	glm::vec3 maxtr = box.max;
	
	glm::vec3 maxtl = box.max;
	maxtl.x = box.min.x;

	glm::vec3 maxbr = maxtr;
	maxbr.y = box.min.y;

	glm::vec3 maxbl = maxtl;
	maxbl.x = box.min.x;

	glm::vec3 minbl = maxbl;
	minbl.z = box.min.z;

	glm::vec3 minbr = maxbr;
	minbr.z = box.min.z;

	glm::vec3 mintl = maxtl;
	maxtl.z = box.min.z;

	glm::vec3 mintr = maxtr;
	mintr.z = box.min.z;

	std::vector<glm::vec4> vertices = 
	{ 
		glm::vec4(mintr, 1.0f),
		glm::vec4(mintl, 1.0f), 
		glm::vec4(minbr, 1.0f), 
		glm::vec4(minbl, 1.0f), 
		glm::vec4(maxtr, 1.0f), 
		glm::vec4(maxtl, 1.0f), 
		glm::vec4(maxbr, 1.0f),
		glm::vec4(maxbl, 1.0f)
	};
	
	BoundingBox bb;
	
	// Transform and update aabb.
	for (int i = 0; i < 8; i++)
	{
		glm::vec3 v = transform * vertices[i];
		bb.min = glm::min(v, bb.min);
		bb.max = glm::max(v, bb.max);
	}

	box = bb;
}

ToolKit::PlaneEquation ToolKit::PlaneFrom(glm::vec3 const pnts[3])
{
	// Expecting 3 non coplanar points in CCW order.
	glm::vec3 v1 = pnts[1] - pnts[0];
	glm::vec3 v2 = pnts[2] - pnts[0];
	
	PlaneEquation eq;
	eq.normal = glm::normalize(glm::cross(v1, v2));
	eq.d = -glm::dot(eq.normal, pnts[0]);

	return eq;
}

ToolKit::PlaneEquation ToolKit::PlaneFrom(glm::vec3 point, glm::vec3 normal)
{
	return { normal, -glm::dot(point, normal) };
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
