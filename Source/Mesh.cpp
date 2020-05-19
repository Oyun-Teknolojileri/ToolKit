#include "stdafx.h"
#include "ToolKit.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "Skeleton.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include <unordered_map>
#include "DebugNew.h"

namespace ToolKit
{

	Mesh::Mesh()
	{
		m_material = MaterialPtr(new Material());
	}

	Mesh::Mesh(String file)
	{
		m_file = file;
		m_material = MaterialPtr(new Material());
	}

	Mesh::~Mesh()
	{
		UnInit();
	}

	void Mesh::Init(bool flushClientSideArray)
	{
		if (m_initiated)
		{
			return;
		}

		InitVertices(flushClientSideArray);
		InitIndices(flushClientSideArray);
		m_material->Init();

		for (MeshPtr mesh : m_subMeshes)
		{
			mesh->Init(flushClientSideArray);
		}

		m_initiated = true;
	}

	void Mesh::UnInit()
	{
		GLuint buffers[2] = { m_vboIndexId, m_vboVertexId };
		glDeleteBuffers(2, buffers);

		for (MeshPtr& subMesh : m_subMeshes)
		{
			subMesh = nullptr;
		}

		m_initiated = false;
	}

	void Mesh::Load()
	{
		if (m_loaded)
		{
			return;
		}

		rapidxml::file<> file(m_file.c_str());
		rapidxml::xml_document<> doc;
		doc.parse<0>(file.data());

		rapidxml::xml_node<>* node = doc.first_node("meshContainer");
		if (node == nullptr)
		{
			return;
		}

		m_aabb = BoundingBox();

		Mesh* mesh = this;
		for (node = node->first_node("mesh"); node; node = node->next_sibling("mesh"))
		{
			if (mesh == nullptr)
			{
				mesh = new Mesh();
				m_subMeshes.push_back(MeshPtr(mesh));
			}

			rapidxml::xml_node<>* materialNode = node->first_node("material");
			String matFile = materialNode->first_attribute("name")->value();

			if (CheckFile(MaterialPath(matFile)))
			{
				mesh->m_material = GetMaterialManager()->Create(MaterialPath(matFile));
			}
			else
			{
				mesh->m_material = GetMaterialManager()->Create(MaterialPath("default.material"));
			}

			rapidxml::xml_node<>* vertex = node->first_node("vertices");
			for (rapidxml::xml_node<>* v = vertex->first_node("v"); v; v = v->next_sibling())
			{
				Vertex vd;
				ExtractXYZFromNode(v->first_node("p"), vd.pos);
				UpdateAABB(vd.pos);

				ExtractXYZFromNode(v->first_node("n"), vd.norm);
				ExtractXYFromNode(v->first_node("t"), vd.tex);
				ExtractXYZFromNode(v->first_node("bt"), vd.btan);
				mesh->m_clientSideVertices.push_back(vd);
			}

			rapidxml::xml_node<>* faces = node->first_node("faces");
			for (rapidxml::xml_node<>* i = faces->first_node("f"); i; i = i->next_sibling())
			{
				glm::ivec3 indices;
				ExtractXYZFromNode(i, indices);
				mesh->m_clientSideIndices.push_back(indices.x);
				mesh->m_clientSideIndices.push_back(indices.y);
				mesh->m_clientSideIndices.push_back(indices.z);
			}

			mesh->m_loaded = true;
			mesh = nullptr;
		}
	}

	Mesh* Mesh::GetCopy()
	{
		Mesh* cpy = new Mesh();
		cpy->m_clientSideVertices = m_clientSideVertices;
		cpy->m_clientSideIndices = m_clientSideIndices;

		// No copy video memory.
		cpy->m_vboVertexId = m_vboVertexId;
		cpy->m_vboIndexId = m_vboIndexId;

		cpy->m_vertexCount = m_vertexCount;
		cpy->m_indexCount = m_indexCount;

		cpy->m_material = std::shared_ptr<Material>(m_material->GetCopy());
		cpy->m_aabb = m_aabb;

		cpy->m_file = m_file;
		cpy->m_initiated = m_initiated;
		cpy->m_loaded = m_loaded;

		for (MeshPtr child : m_subMeshes)
		{
			MeshPtr ccpy = std::shared_ptr<Mesh>(child->GetCopy());
			cpy->m_subMeshes.push_back(ccpy);
		}

		return cpy;
	}

	int Mesh::GetVertexSize()
	{
		return sizeof(Vertex);
	}

	bool Mesh::IsSkinned()
	{
		return false;
	}

	void Mesh::CalculateAABoundingBox()
	{
		if (m_clientSideVertices.empty())
		{
			return;
		}

		m_aabb = BoundingBox();

		MeshRawPtrArray meshes;
		GetAllMeshes(meshes);

		for (size_t i = 0; i < meshes.size(); i++)
		{
			Mesh* m = meshes[i];
			if (m->m_clientSideVertices.empty())
			{
				continue;
			}

			for (size_t j = 0; j < m->m_clientSideVertices.size(); j++)
			{
				Vertex& v = m->m_clientSideVertices[j];
				UpdateAABB(v.pos);
			}
		}
	}

	void Mesh::GetAllMeshes(MeshRawPtrArray& meshes)
	{
		meshes.push_back(this);
		for (size_t i = 0; i < m_subMeshes.size(); i++)
		{
			m_subMeshes[i]->GetAllMeshes(meshes);
		}
	}

	void Mesh::InitVertices(bool flush)
	{
		glDeleteBuffers(1, &m_vboVertexId);

		if (!m_clientSideVertices.empty())
		{
			glGenBuffers(1, &m_vboVertexId);
			glBindBuffer(GL_ARRAY_BUFFER, m_vboVertexId);
			glBufferData(GL_ARRAY_BUFFER, GetVertexSize() * m_clientSideVertices.size(), m_clientSideVertices.data(), GL_STATIC_DRAW);
			m_vertexCount = (uint)m_clientSideVertices.size();
		}

		m_vertexCount = (uint)m_clientSideVertices.size();
		if (flush)
		{
			m_clientSideVertices.clear();
		}
	}

	void Mesh::InitIndices(bool flush)
	{
		glDeleteBuffers(1, &m_vboIndexId);

		if (!m_clientSideIndices.empty())
		{
			glGenBuffers(1, &m_vboIndexId);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboIndexId);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * m_clientSideIndices.size(), m_clientSideIndices.data(), GL_STATIC_DRAW);
			m_indexCount = (uint)m_clientSideIndices.size();
		}

		m_indexCount = (uint)m_clientSideIndices.size();
		if (flush)
		{
			m_clientSideIndices.clear();
		}
	}

	void Mesh::UpdateAABB(const Vec3& v)
	{
		m_aabb.max = glm::max(m_aabb.max, v);
		m_aabb.min = glm::min(m_aabb.min, v);
	}

	SkinMesh::SkinMesh()
	{
		m_skeleton = new Skeleton();
	}

	SkinMesh::SkinMesh(String file)
		: Mesh(file)
	{
		String skelFile = file.substr(0, file.find_last_of("."));
		skelFile += ".skeleton";

		m_skeleton = new Skeleton(skelFile);
	}

	SkinMesh::~SkinMesh()
	{
		UnInit();
	}

	void SkinMesh::Init(bool flushClientSideArray)
	{
		m_skeleton->Init(flushClientSideArray);
		Mesh::Init(flushClientSideArray);
	}

	void SkinMesh::UnInit()
	{
		SafeDel(m_skeleton);
		m_initiated = false;
	}

	void SkinMesh::Load()
	{
		// Skeleton
		m_skeleton->Load();
		assert(m_skeleton->m_loaded);
		if (!m_skeleton->m_loaded)
		{
			return;
		}

		if (m_loaded)
		{
			return;
		}

		rapidxml::file<> file(m_file.c_str());
		rapidxml::xml_document<> doc;
		doc.parse<0>(file.data());

		rapidxml::xml_node<>* node = doc.first_node("meshContainer");
		assert(m_skeleton->m_loaded);
		if (node == nullptr)
		{
			return;
		}

		SkinMesh* mesh = this;
		for (node = node->first_node("skinMesh"); node; node = node->next_sibling("skinMesh"))
		{
			if (mesh == nullptr)
			{
				mesh = new SkinMesh();
				m_subMeshes.push_back(MeshPtr(mesh));
			}

			rapidxml::xml_node<>* materialNode = node->first_node("material");
			String matFile = materialNode->first_attribute("name")->value();

			if (CheckFile(MaterialPath(matFile)))
			{
				mesh->m_material = GetMaterialManager()->Create(MaterialPath(matFile));
			}
			else
			{
				mesh->m_material = GetMaterialManager()->Create(MaterialPath("default.material"));
			}

			rapidxml::xml_node<>* vertex = node->first_node("vertices");
			for (rapidxml::xml_node<>* v = vertex->first_node("v"); v; v = v->next_sibling())
			{
				SkinVertex vd;
				ExtractXYZFromNode(v->first_node("p"), vd.pos);
				ExtractXYZFromNode(v->first_node("n"), vd.norm);
				ExtractXYFromNode(v->first_node("t"), vd.tex);
				ExtractXYZFromNode(v->first_node("bt"), vd.btan);
				ExtractWXYZFromNode(v->first_node("b"), vd.bones);
				ExtractWXYZFromNode(v->first_node("w"), vd.weights);
				mesh->m_clientSideVertices.push_back(vd);
			}

			rapidxml::xml_node<>* faces = node->first_node("faces");
			for (rapidxml::xml_node<>* i = faces->first_node("f"); i; i = i->next_sibling())
			{
				glm::ivec3 indices;
				ExtractXYZFromNode(i, indices);
				mesh->m_clientSideIndices.push_back(indices.x);
				mesh->m_clientSideIndices.push_back(indices.y);
				mesh->m_clientSideIndices.push_back(indices.z);
			}

			mesh = nullptr;
		}

		m_loaded = true;
	}

	int SkinMesh::GetVertexSize()
	{
		return sizeof(SkinVertex);
	}

	bool SkinMesh::IsSkinned()
	{
		return true;
	}

	void SkinMesh::InitVertices(bool flush)
	{
		glDeleteBuffers(1, &m_vboIndexId);

		if (!m_clientSideVertices.empty())
		{
			glGenBuffers(1, &m_vboVertexId);
			glBindBuffer(GL_ARRAY_BUFFER, m_vboVertexId);
			glBufferData(GL_ARRAY_BUFFER, GetVertexSize() * m_clientSideVertices.size(), m_clientSideVertices.data(), GL_STATIC_DRAW);
			m_vertexCount = (uint)m_clientSideVertices.size();
		}

		if (flush)
		{
			m_clientSideVertices.clear();
		}
	}

}
