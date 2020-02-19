#include "stdafx.h"
#include "Grid.h"
#include "Mesh.h"
#include "ToolKit.h"
#include "Material.h"
#include "Texture.h"

ToolKit::Editor::Axis3d::Axis3d()
{
	for (int i = 0; i < 3; i++)
	{
		Arrow2d::ArrowType t;
		switch (i)
		{
		case 0:
			t = Arrow2d::ArrowType::X;
			break;
		case 1:
			t = Arrow2d::ArrowType::Y;
			break;
		case 2:
			t = Arrow2d::ArrowType::Z;
			break;
		}

		Arrow2d arrow(t);
		arrow.m_mesh->m_material->GetRenderState()->depthTestEnabled = false;
		if (i == 0)
		{
			m_mesh = arrow.m_mesh;
		}
		else
		{
			m_mesh->m_subMeshes.push_back(arrow.m_mesh);
		}
	}
}

const std::string g_GRID_MATERIAL_NAME("TK_EDITOR_GRID");

ToolKit::Editor::Grid::Grid(uint size)
{
	// Create grid material.
	if (!Main::GetInstance()->m_materialManager.Exist(g_GRID_MATERIAL_NAME))
	{
		m_material = std::shared_ptr<Material>(new Material());
		m_material->m_diffuseTexture = Main::GetInstance()->m_textureMan.Create(TexturePath("grid.png"));
		m_material->GetRenderState()->blendFunction = BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
		m_material->GetRenderState()->backCullingEnabled = false;
		Main::GetInstance()->m_materialManager.m_storage[g_GRID_MATERIAL_NAME] = m_material;
	}

	// Create grid mesh.
	m_size = size % 2 == 0 ? size : size + 1;
	Resize(size);
}

void ToolKit::Editor::Grid::Resize(uint size)
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

	std::vector<ToolKit::Vertex> vertices;
	vertices.resize(2);

	// x - z lines.
	for (int i = 0; i < 2; i++)
	{
		glm::vec3 p1(-scale, 0.01f, 0.0f);
		glm::vec3 p2(scale, 0.01f, 0.0f);
		glm::vec3 col(1.0f, 0.0f, 0.0f);

		if (i == 1)
		{
			p1 = p1.zyx;
			p2 = p2.zyx;
			col = col.bgr;
		}

		vertices[0].pos = p1;
		vertices[1].pos = p2;

		Material* newMaterial = Main::GetInstance()->m_materialManager.Create(MaterialPath("LineColor.material"))->GetCopy();
		newMaterial->m_color = col;

		std::shared_ptr<Mesh> subMesh(new Mesh());
		subMesh->m_clientSideVertices = vertices;
		subMesh->m_material = std::shared_ptr<Material>(newMaterial);
		m_mesh->m_subMeshes.push_back(subMesh);
		m_mesh->CalculateAABoundingBox();
	}
}
