#include "stdafx.h"

#include "ImGui/imgui.h"

#include "Grid.h"
#include "GlobalDef.h"
#include "ToolKit.h"
#include "Primative.h"
#include "DebugNew.h"

namespace ToolKit
{
	namespace Editor
	{

		Grid::Grid(uint size)
		{
			// Create grid material.
			if (!GetMaterialManager()->Exist(g_gridMaterialName))
			{
				m_material = GetMaterialManager()->GetCopyOfUnlitMaterial();
				m_material->UnInit();
				m_material->m_diffuseTexture = GetTextureManager()->Create(TexturePath("grid.png"));
				m_material->GetRenderState()->blendFunction = BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
				m_material->GetRenderState()->cullMode = CullingType::TwoSided;
				m_material->Init();

				GetMaterialManager()->m_storage[g_gridMaterialName] = m_material;
			}
			else
			{
				m_material = GetMaterialManager()->Create(g_gridMaterialName);
			}

			// Create grid mesh.
			m_size = size % 2 == 0 ? size : size + 1;
			Resize(size);
		}

		void Grid::Resize(uint size)
		{
			m_mesh->UnInit();

			Quad quad;
			float scale = (float)m_size;
			MeshPtr mesh = quad.m_mesh;
			for (int j = 0; j < 4; j++)
			{
				mesh->m_clientSideVertices[j].pos = (mesh->m_clientSideVertices[j].pos * scale).xzy;
				mesh->m_clientSideVertices[j].tex *= scale;
			}
			m_mesh = mesh;
			m_mesh->m_material = m_material;

			VertexArray vertices;
			vertices.resize(2);

			// x - z lines.
			for (int i = 0; i < 2; i++)
			{
				Vec3 p1(-scale * 0.5f, 0.01f, 0.0f);
				Vec3 p2(scale * 0.5f, 0.01f, 0.0f);
				Vec3 col = g_gridAxisRed;

				if (i == 1)
				{
					p1 = p1.zyx;
					p2 = p2.zyx;
					col = g_gridAxisBlue;
				}

				vertices[0].pos = p1;
				vertices[1].pos = p2;

				MaterialPtr newMaterial = GetMaterialManager()->GetCopyOfSolidMaterial();
				newMaterial->GetRenderState()->lineWidth = 3.0f;
				newMaterial->GetRenderState()->drawType = DrawType::Line;
				newMaterial->m_color = col;

				MeshPtr subMesh(new Mesh());
				subMesh->m_clientSideVertices = vertices;
				subMesh->m_material = newMaterial;
				m_mesh->m_subMeshes.push_back(subMesh);
				m_mesh->CalculateAABoundingBox();
			}
		}

		bool Grid::HitTest(const Ray& ray, Vec3& pos)
		{
			Mat4 ts = m_node->GetTransform(TransformationSpace::TS_WORLD);
			Mat4 its = glm::inverse(ts);
			Ray rayInObjectSpace =
			{
				its * Vec4(ray.position, 1.0f),
				its * Vec4(ray.direction, 0.0f)
			};

			float dist = 0.0f;
			if (RayBoxIntersection(rayInObjectSpace, GetAABB(), dist))
			{
				pos = PointOnRay(rayInObjectSpace, dist);
				return true;
			}

			return false;
		}

	}
}
