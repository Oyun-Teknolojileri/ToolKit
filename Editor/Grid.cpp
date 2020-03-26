#include "stdafx.h"
#include "Grid.h"
#include "Mesh.h"
#include "ToolKit.h"
#include "Material.h"
#include "Texture.h"
#include "Surface.h"
#include "Directional.h"
#include "Node.h"
#include "Renderer.h"
#include "Material.h"
#include "GlobalDef.h"
#include "DebugNew.h"

ToolKit::Editor::Cursor::Cursor()
{
	m_billboard = new Drawable();
	Generate();
}

ToolKit::Editor::Cursor::~Cursor()
{
	SafeDel(m_billboard);
}

void ToolKit::Editor::Cursor::LookAt(Camera* cam)
{
	// Billboard placement.
	glm::vec3 cdir, dir;
	cam->GetLocalAxis(cdir, dir, dir);
	dir = glm::normalize(m_pickPosition - cam->m_node->m_translation);
	
	float distToCameraPlane = 10.0f / glm::dot(cdir, dir);
	if (distToCameraPlane < 0)
	{
		return;
	}

	m_billboard->m_node->m_translation = cam->m_node->m_translation + dir * distToCameraPlane;
	m_billboard->m_node->m_orientation = cam->m_node->m_orientation;

	m_node->m_translation = m_billboard->m_node->m_translation;
}

void ToolKit::Editor::Cursor::Generate()
{
	m_mesh->UnInit();

	// Billboard
	Quad quad;
	m_billboard->m_mesh = quad.m_mesh;
	std::shared_ptr<Mesh> meshPtr = m_billboard->m_mesh;

	meshPtr->m_material = std::shared_ptr<Material>(meshPtr->m_material->GetCopy());
	meshPtr->m_material->UnInit();
	meshPtr->m_material->m_diffuseTexture = ToolKit::Main::GetInstance()->m_textureMan.Create(ToolKit::TexturePath("Icons/cursor4k.png"));
	meshPtr->m_material->GetRenderState()->blendFunction = ToolKit::BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
	meshPtr->m_material->Init();

	meshPtr->m_material->GetRenderState()->depthTestEnabled = false;

	// Lines
	std::vector<ToolKit::Vertex> vertices;
	vertices.resize(12);

	vertices[0].pos.z = -0.3f;
	vertices[1].pos.z = -0.7f;

	vertices[2].pos.z = 0.3f;
	vertices[3].pos.z = 0.7f;

	vertices[4].pos.x = 0.3f;
	vertices[5].pos.x = 0.7f;

	vertices[6].pos.x = -0.3f;
	vertices[7].pos.x = -0.7f;

	vertices[8].pos.y = 0.3f;
	vertices[9].pos.y = 0.7f;

	vertices[10].pos.y = -0.3f;
	vertices[11].pos.y = -0.7f;

	Material* newMaterial = Main::GetInstance()->m_materialManager.Create(MaterialPath("LineColor.material"))->GetCopy();
	newMaterial->m_color = glm::vec3(0.1f, 0.1f, 0.1f);
	newMaterial->GetRenderState()->depthTestEnabled = false;

	m_mesh->m_clientSideVertices = vertices;
	m_mesh->m_material = std::shared_ptr<Material>(newMaterial);

	m_mesh->CalculateAABoundingBox();
}

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

ToolKit::Editor::Grid::Grid(uint size)
{
	// Create grid material.
	if (!Main::GetInstance()->m_materialManager.Exist(g_GRID_MATERIAL_NAME))
	{
		m_material = std::shared_ptr<Material>(new Material());
		m_material->m_diffuseTexture = Main::GetInstance()->m_textureMan.Create(TexturePath("grid.png"));
		m_material->GetRenderState()->blendFunction = BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
		m_material->GetRenderState()->cullMode = CullingType::TwoSided;
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

		Material* newMaterial = Main::GetInstance()->m_materialManager.Create(MaterialPath("LineColor.material"))->GetCopy();
		newMaterial->GetRenderState()->lineWidth = 3.0f;
		newMaterial->m_color = col;

		std::shared_ptr<Mesh> subMesh(new Mesh());
		subMesh->m_clientSideVertices = vertices;
		subMesh->m_material = std::shared_ptr<Material>(newMaterial);
		m_mesh->m_subMeshes.push_back(subMesh);
		m_mesh->CalculateAABoundingBox();
	}
}
