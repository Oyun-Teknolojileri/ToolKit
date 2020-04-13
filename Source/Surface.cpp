#include "stdafx.h"
#include "Surface.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "Node.h"
#include "DebugNew.h"

namespace ToolKit
{

	Surface::Surface(std::shared_ptr<Texture> texture, Vec2 pivotOffset)
	{
		m_mesh->m_material->m_diffuseTexture = texture;
		m_pivotOffset = pivotOffset;
		CreateQuat();
		m_loaded = true;
	}

	Surface::Surface(std::shared_ptr<Texture> texture, const std::vector<Vertex>& vertices)
	{
		m_mesh->m_material->m_diffuseTexture = texture;
		m_mesh->m_clientSideVertices = vertices;
		m_loaded = true;
	}

	Surface::Surface(String file, Vec2 pivotOffset)
	{
		m_file = file;
		m_pivotOffset = pivotOffset;
	}

	Surface::~Surface()
	{
		UnInit();
	}

	EntityType Surface::GetType() const
	{
		return EntityType::Entity_Surface;
	}

	void Surface::Load()
	{
		if (m_loaded)
			return;

		assert(!m_file.empty());
		m_mesh->m_material->m_diffuseTexture = GetTextureManager()->Create(m_file);
		CreateQuat();

		m_loaded = true;
	}

	void Surface::Init(bool flushClientSideArray)
	{
		if (m_initiated)
			return;

		m_mesh->m_material->m_diffuseTexture->Init(flushClientSideArray);
		m_mesh->Init(flushClientSideArray);

		m_mesh->m_material->GetRenderState()->blendFunction = BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
		m_mesh->m_material->GetRenderState()->depthTestEnabled = false;

		m_initiated = true;
	}

	void Surface::UnInit()
	{
		m_initiated = false;
	}

	void Surface::CreateQuat()
	{
		float width = (float)m_mesh->m_material->m_diffuseTexture->m_width;
		float height = (float)m_mesh->m_material->m_diffuseTexture->m_height;
		float depth = 0;
		Vec2 absOffset = Vec2(m_pivotOffset.x * width, m_pivotOffset.y * height);

		std::vector<Vertex> vertices;
		vertices.resize(6);
		vertices[0].pos = Vec3(-absOffset.x, -absOffset.y, depth);
		vertices[0].tex = Vec2(0.0f, 1.0f);
		vertices[1].pos = Vec3(width - absOffset.x, -absOffset.y, depth);
		vertices[1].tex = Vec2(1.0f, 1.0f);
		vertices[2].pos = Vec3(-absOffset.x, height - absOffset.y, depth);
		vertices[2].tex = Vec2(0.0f, 0.0f);

		vertices[3].pos = Vec3(width - absOffset.x, -absOffset.y, depth);
		vertices[3].tex = Vec2(1.0f, 1.0f);
		vertices[4].pos = Vec3(width - absOffset.x, height - absOffset.y, depth);
		vertices[4].tex = Vec2(1.0f, 0.0f);
		vertices[5].pos = Vec3(-absOffset.x, height - absOffset.y, depth);
		vertices[5].tex = Vec2(0.0f, 0.0f);

		m_mesh->m_clientSideVertices = vertices;
	}

}
