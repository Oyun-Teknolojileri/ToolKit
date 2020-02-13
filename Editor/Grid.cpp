#include "stdafx.h"
#include "Grid.h"
#include "Mesh.h"
#include "ToolKit.h"
#include "Material.h"
#include "Texture.h"

const std::string g_GRID_MATERIAL_NAME("TK_EDITOR_GRID");

ToolKit::Editor::Grid::Grid(uint size)
{
	// Create grid material.
	if (!Main::GetInstance()->m_materialManager.Exist(g_GRID_MATERIAL_NAME))
	{
		std::shared_ptr<Material> gridMaterial = std::shared_ptr<Material>(new Material());
		gridMaterial->m_diffuseTexture = Main::GetInstance()->m_textureMan.Create(TexturePath("grid.png"));
		gridMaterial->GetRenderState()->blendFunction = BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
		gridMaterial->GetRenderState()->backCullingEnabled = false;
		Main::GetInstance()->m_materialManager.m_storage[g_GRID_MATERIAL_NAME] = gridMaterial;
	}

	// Create grid mesh.
	m_size = size % 2 == 0 ? size : size + 1;
	Resize(size);
}

void ToolKit::Editor::Grid::Resize(uint size)
{
	m_mesh->UnInit();

	// Create 4 different quad to complate the grid.
	glm::vec3 shiftVecs[4] =
	{
		glm::vec3(0.5f, 0.5f, 0.0), // 1'th quadrant and goes to the 4'th quadrant.
		glm::vec3(-0.5f, 0.5f, 0.0),
		glm::vec3(-0.5f, -0.5f, 0.0),
		glm::vec3(0.5f, -0.5f, 0.0)
	};

	Quad quads[4];
	float scale = m_size / 2.0f;
	for (int i = 0; i < 4; i++)
	{
		std::shared_ptr<Mesh> mesh = quads[i].m_mesh;
		for (int j = 0; j < 4; j++)
		{
			mesh->m_clientSideVertices[j].pos = ((mesh->m_clientSideVertices[j].pos + shiftVecs[j]) * scale).xzy;
			mesh->m_clientSideVertices[j].tex *= scale;
		}

		if (i == 0)
		{
			m_mesh->m_clientSideIndices.swap(mesh->m_clientSideIndices);
			m_mesh->m_clientSideVertices.swap(mesh->m_clientSideVertices);
			m_mesh->m_material = Main::GetInstance()->m_materialManager.Create(g_GRID_MATERIAL_NAME);
		}
		else
		{
			Mesh* subMesh = new Mesh();
			subMesh->m_clientSideIndices.swap(mesh->m_clientSideIndices);
			subMesh->m_clientSideVertices.swap(mesh->m_clientSideVertices);
			m_mesh->m_subMeshes.push_back(subMesh);
		}
	}
}
