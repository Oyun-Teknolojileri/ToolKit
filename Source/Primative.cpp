#include "stdafx.h"
#include "Primative.h"
#include "Mesh.h"
#include "ToolKit.h"
#include "MathUtil.h"
#include "Directional.h"
#include "Node.h"
#include "DebugNew.h"

namespace ToolKit
{

	Billboard::Billboard(const Settings& settings)
		: m_settings(settings)
	{
	}

	EntityType Billboard::GetType() const
	{
		return EntityType::Entity_Billboard;
	}

	void Billboard::LookAt(Camera* cam)
	{
		// SetTranslation in given space is not provided in Node class.
		// Therefore all objects must be in the worldSpace.
		assert(m_node->m_parent == nullptr);
		assert(cam->m_node->m_parent == nullptr);

		Camera::CamData data = cam->GetData();

		// Billboard placement.
		if (m_settings.distanceToCamera > 0.0f)
		{
			Vec3 cdir, dir;
			cam->GetLocalAxis(cdir, dir, dir);
			dir = glm::normalize(m_worldLocation - cam->m_node->m_translation);

			float radialToPlanarDistance = 1.0f / glm::dot(cdir, dir); // Always place at the same distance from the near plane.
			if (radialToPlanarDistance < 0)
			{
				return;
			}

			m_node->m_translation = cam->m_node->m_translation + dir * m_settings.distanceToCamera * radialToPlanarDistance;
		}

		if (m_settings.heightInScreenSpace > 0.0f)
		{
			m_node->m_scale = Vec3(m_settings.heightInScreenSpace / data.height); // Compensate shrinkage due to height changes.
		}

		if (m_settings.lookAtCamera)
		{
			m_node->m_orientation = cam->m_node->m_orientation;
		}
	}

	Cube::Cube()
	{
		Generate(Vec3(1.0f));
	}

	Cube::Cube(const Vec3& scale)
	{
		Generate(scale);
	}

	void Cube::Generate(Vec3 scale)
	{
		std::vector<Vertex> vertices;
		vertices.resize(36);

		Vec3 corners[8]
		{
			Vec3(-0.5f, 0.5f, 0.5f) * scale, // FTL.
			Vec3(-0.5f, -0.5f, 0.5f) * scale, // FBL.
			Vec3(0.5f, -0.5f, 0.5f)* scale, // FBR.
			Vec3(0.5f, 0.5f, 0.5f) * scale, // FTR.
			Vec3(-0.5f, 0.5f, -0.5f)* scale, // BTL.
			Vec3(-0.5f, -0.5f, -0.5f)* scale, // BBL.
			Vec3(0.5f, -0.5f, -0.5f)* scale, // BBR.
			Vec3(0.5f, 0.5f, -0.5f) * scale, // BTR.
		};

		// Front
		vertices[0].pos = corners[0];
		vertices[0].tex = Vec2(0.0f, 1.0f);
		vertices[0].norm = Vec3(0.0f, 0.0f, 1.0f);
		vertices[1].pos = corners[1];
		vertices[1].tex = Vec2(0.0f, 0.0f);
		vertices[1].norm = Vec3(0.0f, 0.0f, 1.0f);
		vertices[2].pos = corners[2];
		vertices[2].tex = Vec2(1.0f, 0.0f);
		vertices[2].norm = Vec3(0.0f, 0.0f, 1.0f);

		vertices[3].pos = corners[0];
		vertices[3].tex = Vec2(0.0f, 1.0f);
		vertices[3].norm = Vec3(0.0f, 0.0f, 1.0f);
		vertices[4].pos = corners[2];
		vertices[4].tex = Vec2(1.0f, 0.0f);
		vertices[4].norm = Vec3(0.0f, 0.0f, 1.0f);
		vertices[5].pos = corners[3];
		vertices[5].tex = Vec2(1.0f, 1.0f);
		vertices[5].norm = Vec3(0.0f, 0.0f, 1.0f);

		// Right
		vertices[6].pos = corners[3];
		vertices[6].tex = Vec2(0.0f, 1.0f);
		vertices[6].norm = Vec3(1.0f, 0.0f, 0.0f);
		vertices[7].pos = corners[2];
		vertices[7].tex = Vec2(0.0f, 0.0f);
		vertices[7].norm = Vec3(1.0f, 0.0f, 0.0f);
		vertices[8].pos = corners[6];
		vertices[8].tex = Vec2(1.0f, 0.0f);
		vertices[8].norm = Vec3(1.0f, 0.0f, 0.0f);

		vertices[9].pos = corners[3];
		vertices[9].tex = Vec2(0.0f, 1.0f);
		vertices[9].norm = Vec3(1.0f, 0.0f, 0.0f);
		vertices[10].pos = corners[6];
		vertices[10].tex = Vec2(1.0f, 0.0f);
		vertices[10].norm = Vec3(1.0f, 0.0f, 0.0f);
		vertices[11].pos = corners[7];
		vertices[11].tex = Vec2(1.0f, 1.0f);
		vertices[11].norm = Vec3(1.0f, 0.0f, 0.0f);

		// Top
		vertices[12].pos = corners[0];
		vertices[12].tex = Vec2(0.0f, 0.0f);
		vertices[12].norm = Vec3(0.0f, 1.0f, 0.0f);
		vertices[13].pos = corners[3];
		vertices[13].tex = Vec2(1.0f, 0.0f);
		vertices[13].norm = Vec3(0.0f, 1.0f, 0.0f);
		vertices[14].pos = corners[7];
		vertices[14].tex = Vec2(1.0f, 1.0f);
		vertices[14].norm = Vec3(0.0f, 1.0f, 0.0f);

		vertices[15].pos = corners[0];
		vertices[15].tex = Vec2(0.0f, 0.0f);
		vertices[15].norm = Vec3(0.0f, 1.0f, 0.0f);
		vertices[16].pos = corners[7];
		vertices[16].tex = Vec2(1.0f, 1.0f);
		vertices[16].norm = Vec3(0.0f, 1.0f, 0.0f);
		vertices[17].pos = corners[4];
		vertices[17].tex = Vec2(0.0f, 1.0f);
		vertices[17].norm = Vec3(0.0f, 1.0f, 0.0f);

		// Back
		vertices[18].pos = corners[4];
		vertices[18].tex = Vec2(0.0f, 1.0f);
		vertices[18].norm = Vec3(0.0f, 0.0f, -1.0f);
		vertices[19].pos = corners[6];
		vertices[19].tex = Vec2(1.0f, 0.0f);
		vertices[19].norm = Vec3(0.0f, 0.0f, -1.0f);
		vertices[20].pos = corners[5];
		vertices[20].tex = Vec2(0.0f, 0.0f);
		vertices[20].norm = Vec3(0.0f, 0.0f, -1.0f);

		vertices[21].pos = corners[4];
		vertices[21].tex = Vec2(0.0f, 1.0f);
		vertices[21].norm = Vec3(0.0f, 0.0f, -1.0f);
		vertices[22].pos = corners[7];
		vertices[22].tex = Vec2(1.0f, 1.0f);
		vertices[22].norm = Vec3(0.0f, 0.0f, -1.0f);
		vertices[23].pos = corners[6];
		vertices[23].tex = Vec2(1.0f, 0.0f);
		vertices[23].norm = Vec3(0.0f, 0.0f, -1.0f);

		// Left
		vertices[24].pos = corners[0];
		vertices[24].tex = Vec2(0.0f, 1.0f);
		vertices[24].norm = Vec3(-1.0f, 0.0f, 0.0f);
		vertices[25].pos = corners[5];
		vertices[25].tex = Vec2(1.0f, 0.0f);
		vertices[25].norm = Vec3(-1.0f, 0.0f, 0.0f);
		vertices[26].pos = corners[1];
		vertices[26].tex = Vec2(0.0f, 0.0f);
		vertices[26].norm = Vec3(-1.0f, 0.0f, 0.0f);

		vertices[27].pos = corners[0];
		vertices[27].tex = Vec2(0.0f, 1.0f);
		vertices[27].norm = Vec3(-1.0f, 0.0f, 0.0f);
		vertices[28].pos = corners[4];
		vertices[28].tex = Vec2(1.0f, 1.0f);
		vertices[28].norm = Vec3(-1.0f, 0.0f, 0.0f);
		vertices[29].pos = corners[5];
		vertices[29].tex = Vec2(1.0f, 0.0f);
		vertices[29].norm = Vec3(-1.0f, 0.0f, 0.0f);

		// Bottom
		vertices[30].pos = corners[1];
		vertices[30].tex = Vec2(0.0f, 1.0f);
		vertices[30].norm = Vec3(0.0f, -1.0f, 0.0f);
		vertices[31].pos = corners[6];
		vertices[31].tex = Vec2(1.0f, 0.0f);
		vertices[31].norm = Vec3(0.0f, -1.0f, 0.0f);
		vertices[32].pos = corners[2];
		vertices[32].tex = Vec2(0.0f, 0.0f);
		vertices[32].norm = Vec3(0.0f, -1.0f, 0.0f);

		vertices[33].pos = corners[1];
		vertices[33].tex = Vec2(0.0f, 1.0f);
		vertices[33].norm = Vec3(0.0f, -1.0f, 0.0f);
		vertices[34].pos = corners[5];
		vertices[34].tex = Vec2(1.0f, 1.0f);
		vertices[34].norm = Vec3(0.0f, -1.0f, 0.0f);
		vertices[35].pos = corners[6];
		vertices[35].tex = Vec2(1.0f, 0.0f);
		vertices[35].norm = Vec3(0.0f, -1.0f, 0.0f);

		m_mesh->m_vertexCount = (uint)vertices.size();
		m_mesh->m_clientSideVertices = vertices;
		m_mesh->m_clientSideIndices = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35 };
		m_mesh->m_indexCount = (uint)m_mesh->m_clientSideIndices.size();
		m_mesh->m_material = GetMaterialManager()->GetCopyOfDefaultMaterial();

		m_mesh->CalculateAABoundingBox();
	}

	EntityType Cube::GetType() const
	{
		return EntityType::Entity_Cube;
	}

	Quad::Quad()
	{
		std::vector<Vertex> vertices;
		vertices.resize(4);

		// Front
		vertices[0].pos = Vec3(-0.5f, 0.5f, 0.0f);
		vertices[0].tex = Vec2(0.0f, 1.0f);
		vertices[0].norm = Vec3(0.0f, 0.0f, 1.0f);
		vertices[0].btan = Vec3(0.0f, 1.0f, 0.0f);

		vertices[1].pos = Vec3(-0.5f, -0.5f, 0.0f);
		vertices[1].tex = Vec2(0.0f, 0.0f);
		vertices[1].norm = Vec3(0.0f, 0.0f, 1.0f);
		vertices[1].btan = Vec3(0.0f, 1.0f, 0.0f);

		vertices[2].pos = Vec3(0.5f, -0.5f, 0.0f);
		vertices[2].tex = Vec2(1.0f, 0.0f);
		vertices[2].norm = Vec3(0.0f, 0.0f, 1.0f);
		vertices[2].btan = Vec3(0.0f, 1.0f, 0.0f);

		vertices[3].pos = Vec3(0.5f, 0.5f, 0.0f);
		vertices[3].tex = Vec2(1.0f, 1.0f);
		vertices[3].norm = Vec3(0.0f, 0.0f, 1.0f);
		vertices[3].btan = Vec3(0.0f, 1.0f, 0.0f);

		m_mesh->m_vertexCount = (uint)vertices.size();
		m_mesh->m_clientSideVertices = vertices;
		m_mesh->m_indexCount = 6;
		m_mesh->m_clientSideIndices = { 0,1,2,0,2,3 };
		m_mesh->m_material = GetMaterialManager()->GetCopyOfDefaultMaterial();

		m_mesh->CalculateAABoundingBox();
	}

	EntityType Quad::GetType() const
	{
		return EntityType::Entity_Quad;
	}

	Sphere::Sphere()
	{
		const float r = 1.0f;
		const int nRings = 32;
		const int nSegments = 32;

		std::vector<Vertex> vertices;
		std::vector<uint> indices;

		constexpr float fDeltaRingAngle = (glm::pi<float>() / nRings);
		constexpr float fDeltaSegAngle = (glm::two_pi<float>() / nSegments);
		unsigned short wVerticeIndex = 0;

		// Generate the group of rings for the sphere
		for (int ring = 0; ring <= nRings; ring++)
		{
			float r0 = r * sinf(ring * fDeltaRingAngle);
			float y0 = r * cosf(ring * fDeltaRingAngle);

			// Generate the group of segments for the current ring
			for (int seg = 0; seg <= nSegments; seg++)
			{
				float x0 = r0 * sinf(seg * fDeltaSegAngle);
				float z0 = r0 * cosf(seg * fDeltaSegAngle);

				// Add one vertex to the strip which makes up the sphere
				Vertex v;
				v.pos = Vec3(x0, y0, z0);
				v.norm = Vec3(x0, y0, z0);
				v.tex = Vec2((float)seg / (float)nSegments, (float)ring / (float)nRings);

				float r2, zenith, azimuth;
				ToSpherical(v.pos, r2, zenith, azimuth);
				v.btan = Vec3(r * glm::cos(zenith) * glm::sin(azimuth), -r * glm::sin(zenith), r * glm::cos(zenith) * glm::cos(azimuth));

				vertices.push_back(v);

				if (ring != nRings)
				{
					// each vertex (except the last) has six indices pointing to it
					indices.push_back(wVerticeIndex + nSegments + 1);
					indices.push_back(wVerticeIndex);
					indices.push_back(wVerticeIndex + nSegments);
					indices.push_back(wVerticeIndex + nSegments + 1);
					indices.push_back(wVerticeIndex + 1);
					indices.push_back(wVerticeIndex);
					wVerticeIndex++;
				}
			} // end for seg
		} // end for ring

		m_mesh->m_vertexCount = (uint)vertices.size();
		m_mesh->m_clientSideVertices = vertices;
		m_mesh->m_indexCount = (uint)indices.size();
		m_mesh->m_clientSideIndices = indices;
		m_mesh->m_material = GetMaterialManager()->GetCopyOfDefaultMaterial();

		m_mesh->CalculateAABoundingBox();
	}

	EntityType Sphere::GetType() const
	{
		return EntityType::Entity_Sphere;
	}

	Cone::Cone()
	{
		Generate(1.0f, 1.0f, 30, 30);
	}

	Cone::Cone(float height, float radius, int nSegBase, int nSegHeight)
	{
		Generate(height, radius, nSegBase, nSegHeight);
	}

	// https://github.com/OGRECave/ogre-procedural/blob/master/library/src/ProceduralConeGenerator.cpp
	void Cone::Generate(float height, float radius, int nSegBase, int nSegHeight)
	{
		std::vector<Vertex> vertices;
		std::vector<uint> indices;

		float deltaAngle = (glm::two_pi<float>() / nSegBase);
		float deltaHeight = height / nSegHeight;
		int offset = 0;

		Vec3 refNormal = glm::normalize(Vec3(radius, height, 0.0f));
		Quaternion q;

		for (int i = 0; i <= nSegHeight; i++)
		{
			float r0 = radius * (1 - i / (float)nSegHeight);
			for (int j = 0; j <= nSegBase; j++)
			{
				float x0 = r0 * glm::cos(j * deltaAngle);
				float z0 = r0 * glm::sin(j * deltaAngle);

				q = glm::angleAxis(glm::radians(-deltaAngle * j), Y_AXIS);

				Vertex v
				{
					Vec3(x0, i * deltaHeight, z0),
					q * refNormal,
					Vec2(j / (float)nSegBase, i / (float)nSegHeight),
					Vec3() // btan missing.
				};

				vertices.push_back(v);

				if (i != nSegHeight && j != nSegBase)
				{
					indices.push_back(offset + nSegBase + 2);
					indices.push_back(offset);
					indices.push_back(offset + nSegBase + 1);
					indices.push_back(offset + nSegBase + +2); // Is this valid "nSegBase + +2" ??
					indices.push_back(offset + 1);
					indices.push_back(offset);
				}

				offset++;
			}
		}

		//low cap
		int centerIndex = offset;

		Vertex v
		{
			Vec3(),
			-Y_AXIS,
			Y_AXIS,
			Vec3() // btan missing.
		};
		vertices.push_back(v);

		offset++;
		for (int j = 0; j <= nSegBase; j++)
		{
			float x0 = radius * glm::cos(j * deltaAngle);
			float z0 = radius * glm::sin(j * deltaAngle);

			Vertex v
			{
				Vec3(x0, 0.0f, z0),
				-Y_AXIS,
				Vec2(j / (float)nSegBase, 0.0f),
				Vec3() // btan missing.
			};
			vertices.push_back(v);

			if (j != nSegBase)
			{
				indices.push_back(centerIndex);
				indices.push_back(offset);
				indices.push_back(offset + 1);
			}
			offset++;
		}

		m_mesh->m_vertexCount = (uint)vertices.size();
		m_mesh->m_clientSideVertices = vertices;
		m_mesh->m_indexCount = (uint)indices.size();
		m_mesh->m_clientSideIndices = indices;
		m_mesh->m_material = GetMaterialManager()->Create(MaterialPath("default.material"));

		m_mesh->CalculateAABoundingBox();
	}

	EntityType Cone::GetType() const
	{
		return EntityType::Entity_Cone;
	}

	Arrow2d::Arrow2d()
	{
		Generate();
	}

	Arrow2d::Arrow2d(AxisLabel label)
		: m_label(label)
	{
		Generate();
	}

	EntityType Arrow2d::GetType() const
	{
		return EntityType::Etity_Arrow;
	}

	void Arrow2d::Generate()
	{
		std::vector<Vertex> vertices;
		vertices.resize(8);

		// Line
		vertices[0].pos = Vec3(0.0f, 0.0f, 0.0f);
		vertices[1].pos = Vec3(0.8f, 0.0f, 0.0f);

		// Triangle
		vertices[2].pos = Vec3(0.8f, -0.2f, 0.0f);
		vertices[3].pos = Vec3(0.8f, 0.2f, 0.0f);
		vertices[4].pos = Vec3(0.8f, 0.2f, 0.0f);
		vertices[5].pos = Vec3(1.0f, 0.0f, 0.0f);
		vertices[6].pos = Vec3(1.0f, 0.0f, 0.0f);
		vertices[7].pos = Vec3(0.8f, -0.2f, 0.0f);

		MaterialPtr newMaterial = GetMaterialManager()->GetCopyOfSolidMaterial();
		newMaterial->GetRenderState()->drawType = DrawType::Line;
		newMaterial->m_color = Vec3(0.89f, 0.239f, 0.341f);

		Quaternion rotation;
		if (m_label == AxisLabel::Y)
		{
			newMaterial->m_color = Vec3(0.537f, 0.831f, 0.07f);
			rotation = glm::angleAxis(glm::half_pi<float>(), Z_AXIS);
		}

		if (m_label == AxisLabel::Z)
		{
			newMaterial->m_color = Vec3(0.196f, 0.541f, 0.905f);
			rotation = glm::angleAxis(-glm::half_pi<float>(), Y_AXIS);
		}

		for (size_t i = 0; i < vertices.size(); i++)
		{
			vertices[i].pos = rotation * vertices[i].pos;
		}

		m_mesh->m_vertexCount = (uint)vertices.size();
		m_mesh->m_clientSideVertices = vertices;
		m_mesh->m_material = newMaterial;

		m_mesh->CalculateAABoundingBox();
	}

	LineBatch::LineBatch(const std::vector<Vec3>& linePnts, const Vec3& color, DrawType t, float lineWidth)
	{
		MaterialPtr newMaterial = GetMaterialManager()->GetCopyOfSolidMaterial();
		newMaterial->GetRenderState()->drawType = t;
		m_mesh->m_material = newMaterial;

		Generate(linePnts, color, t, lineWidth);
	}

	EntityType LineBatch::GetType() const
	{
		return EntityType::Entity_LineBatch;
	}

	void LineBatch::Generate(const std::vector<Vec3>& linePnts, const Vec3& color, DrawType t, float lineWidth)
	{
		std::vector<Vertex> vertices;
		vertices.resize(linePnts.size());

		m_mesh->UnInit();

		for (size_t i = 0; i < linePnts.size(); i++)
		{
			vertices[i].pos = linePnts[i];
		}

		m_mesh->m_vertexCount = (uint)vertices.size();
		m_mesh->m_clientSideVertices = vertices;
		m_mesh->m_material->m_color = color;
		m_mesh->m_material->GetRenderState()->lineWidth = lineWidth;

		m_mesh->CalculateAABoundingBox();
	}

}
