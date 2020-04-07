#include "stdafx.h"

#include "Gizmo.h"
#include "ToolKit.h"
#include "Node.h"
#include "Surface.h"
#include "Directional.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "RenderState.h"
#include "Material.h"
#include "Primative.h"
#include "GlobalDef.h"
#include "DebugNew.h"

ToolKit::Editor::Cursor::Cursor()
	: Billboard({ true, 10.0f, 400.0f })
{
	Generate();
}

ToolKit::Editor::Cursor::~Cursor()
{
}

void ToolKit::Editor::Cursor::Generate()
{
	m_mesh->UnInit();

	// Billboard
	Quad quad;
	std::shared_ptr<Mesh> meshPtr = quad.m_mesh;

	meshPtr->m_material = std::shared_ptr<Material>(meshPtr->m_material->GetCopy());
	meshPtr->m_material->UnInit();
	meshPtr->m_material->m_diffuseTexture = ToolKit::Main::GetInstance()->m_textureMan.Create(ToolKit::TexturePath("Icons/cursor4k.png"));
	meshPtr->m_material->GetRenderState()->blendFunction = ToolKit::BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
	meshPtr->m_material->Init();

	meshPtr->m_material->GetRenderState()->depthTestEnabled = false;
	m_mesh->m_subMeshes.push_back(meshPtr);

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
	: Billboard({ false, 10.0f, 400.0f })
{
	Generate();
}

void ToolKit::Editor::Axis3d::Generate()
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

ToolKit::Editor::MoveGizmo::MoveGizmo()
	: Billboard({ false, 10.0f, 400.0f })
{
	m_hitBox[0].min = glm::vec3(0.05f, -0.05f, -0.05f);
	m_hitBox[0].max = glm::vec3(1.0f, 0.05f, 0.05f);

	m_hitBox[1].min = m_hitBox[0].min.yxz;
	m_hitBox[1].max = m_hitBox[0].max.yxz;

	m_hitBox[2].min = m_hitBox[0].min.zyx;
	m_hitBox[2].max = m_hitBox[0].max.zyx;

	Generate();
}

ToolKit::Editor::MoveGizmo::~MoveGizmo()
{
}

ToolKit::Editor::MoveGizmo::Axis ToolKit::Editor::MoveGizmo::HitTest(const Ray& ray)
{
	glm::mat4 invMat = m_node->GetTransform(TransformationSpace::TS_WORLD);
	glm::mat4 tpsMat = glm::transpose(invMat);
	invMat = glm::inverse(invMat);

	Ray rayInObjectSpace = ray;
	rayInObjectSpace.position = invMat * glm::vec4(rayInObjectSpace.position, 1.0f);
	rayInObjectSpace.direction = tpsMat * glm::vec4(rayInObjectSpace.direction, 1.0f);

	float d, t = std::numeric_limits<float>::infinity();
	int minIndx = -1;
	for (int i = 0; i < 3; i++)
	{
		if (RayBoxIntersection(rayInObjectSpace, m_hitBox[i], d))
		{
			if (d < t)
			{
				t = d;
				minIndx = i;
			}
		}
	}

	switch (minIndx)
	{
	case 0:
		return Axis::X;
		break;
	case 1:
		return Axis::Y;
		break;
	case 2:
		return Axis::Z;
		break;
	}

	return Axis::None;
}

void ToolKit::Editor::MoveGizmo::Generate()
{
	Axis3d axis;
	axis.m_mesh->Init();
	
	m_mesh = axis.m_mesh;
	axis.m_mesh = nullptr;
}
