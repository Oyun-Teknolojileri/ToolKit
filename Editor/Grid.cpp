#include "stdafx.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"

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
				m_material = std::shared_ptr<Material>(new Material());
				m_material->m_diffuseTexture = GetTextureManager()->Create(TexturePath("grid.png"));
				m_material->GetRenderState()->blendFunction = BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
				m_material->GetRenderState()->cullMode = CullingType::TwoSided;
				GetMaterialManager()->m_storage[g_gridMaterialName] = m_material;
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
			std::shared_ptr<Mesh> mesh = quad.m_mesh;
			for (int j = 0; j < 4; j++)
			{
				mesh->m_clientSideVertices[j].pos = (mesh->m_clientSideVertices[j].pos * scale).xzy;
				mesh->m_clientSideVertices[j].tex *= scale;
			}
			m_mesh = mesh;
			m_mesh->m_material = m_material;

			std::vector<Vertex> vertices;
			vertices.resize(2);

			// x - z lines.
			for (int i = 0; i < 2; i++)
			{
				glm::vec3 p1(-scale * 0.5f, 0.01f, 0.0f);
				glm::vec3 p2(scale * 0.5f, 0.01f, 0.0f);
				glm::vec3 col = g_gridAxisRed;

				if (i == 1)
				{
					p1 = p1.zyx;
					p2 = p2.zyx;
					col = g_gridAxisBlue;
				}

				vertices[0].pos = p1;
				vertices[1].pos = p2;

				std::shared_ptr<Material> newMaterial = GetMaterialManager()->GetCopyOfSolidMaterial();
				newMaterial->GetRenderState()->lineWidth = 3.0f;
				newMaterial->m_color = col;

				std::shared_ptr<Mesh> subMesh(new Mesh());
				subMesh->m_clientSideVertices = vertices;
				subMesh->m_material = newMaterial;
				m_mesh->m_subMeshes.push_back(subMesh);
				m_mesh->CalculateAABoundingBox();
			}
		}

	}
}
