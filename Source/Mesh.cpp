#include "stdafx.h"
#include "ToolKit.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "Skeleton.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "DebugNew.h"

#include <unordered_map>

namespace ToolKit
{

	Mesh::Mesh()
	{
		m_material = std::make_shared<Material>();
	}

	Mesh::Mesh(String file)
	{
		m_file = file;
		m_material = std::make_shared<Material>();
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
		ConstructFaces();
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
		m_vboVertexId = 0;
		m_vboIndexId = 0;

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

		XmlFile file(m_file.c_str());
		XmlDocument doc;
		doc.parse<0>(file.data());

		XmlNode* node = doc.first_node("meshContainer");
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

			XmlNode* materialNode = node->first_node("material");
			String matFile = materialNode->first_attribute("name")->value();

			if (CheckFile(MaterialPath(matFile)))
			{
				mesh->m_material = GetMaterialManager()->Create(MaterialPath(matFile));
			}
			else
			{
				mesh->m_material = GetMaterialManager()->Create(MaterialPath("default.material"));
			}

			XmlNode* vertex = node->first_node("vertices");
			for (XmlNode* v = vertex->first_node("v"); v; v = v->next_sibling())
			{
				Vertex vd;
				ExtractXYZFromNode(v->first_node("p"), vd.pos);
				UpdateAABB(vd.pos);

				ExtractXYZFromNode(v->first_node("n"), vd.norm);
				ExtractXYFromNode(v->first_node("t"), vd.tex);
				ExtractXYZFromNode(v->first_node("bt"), vd.btan);
				mesh->m_clientSideVertices.push_back(vd);
			}

			XmlNode* faces = node->first_node("faces");
			for (XmlNode* i = faces->first_node("f"); i; i = i->next_sibling())
			{
				glm::ivec3 indices;
				ExtractXYZFromNode(i, indices);
				mesh->m_clientSideIndices.push_back(indices.x);
				mesh->m_clientSideIndices.push_back(indices.y);
				mesh->m_clientSideIndices.push_back(indices.z);
			}

			mesh->m_loaded = true;
			mesh->m_vertexCount = (int)mesh->m_clientSideVertices.size();
			mesh->m_indexCount = (int)mesh->m_clientSideIndices.size();
			mesh = nullptr;
		}
	}

	Mesh* Mesh::GetCopy()
	{
		Mesh* cpy = new Mesh();
		cpy->m_clientSideVertices = m_clientSideVertices;
		cpy->m_vertexCount = m_vertexCount;
		cpy->m_clientSideIndices = m_clientSideIndices;
		cpy->m_indexCount = m_indexCount;

		// Copy video memory.
		if (m_vertexCount > 0)
		{
			glGenBuffers(1, &cpy->m_vboVertexId);
			glBindBuffer(GL_COPY_WRITE_BUFFER, cpy->m_vboVertexId);
			glBindBuffer(GL_COPY_READ_BUFFER, m_vboVertexId);
			uint size = GetVertexSize() * m_vertexCount;
			glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_STATIC_DRAW);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);
		}

		if (m_indexCount > 0)
		{
			glGenBuffers(1, &cpy->m_vboIndexId);
			glBindBuffer(GL_COPY_WRITE_BUFFER, cpy->m_vboIndexId);
			glBindBuffer(GL_COPY_READ_BUFFER, m_vboIndexId);
			uint size = sizeof(uint)* m_indexCount;
			glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_STATIC_DRAW);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);
		}

		cpy->m_material = MaterialPtr(m_material->GetCopy());
		cpy->m_aabb = m_aabb;

		cpy->m_file = m_file;
		cpy->m_initiated = m_initiated;
		cpy->m_loaded = m_loaded;

		for (MeshPtr child : m_subMeshes)
		{
			MeshPtr ccpy = MeshPtr(child->GetCopy());
			cpy->m_subMeshes.push_back(ccpy);
		}

		return cpy;
	}

	int Mesh::GetVertexSize() const
	{
		return sizeof(Vertex);
	}

	bool Mesh::IsSkinned() const
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

	void Mesh::Scale(const Vec3& scale)
	{
		assert(!m_initiated && "Call this before initiating the mesh.");
		if (!m_initiated)
		{
			MeshRawPtrArray meshes;
			GetAllMeshes(meshes);
			m_aabb.max = Vec3(-FLT_MAX);
			m_aabb.min = Vec3(FLT_MAX);
			for (Mesh* mesh : meshes)
			{
				for (size_t i = 0; i < m_vertexCount; i++)
				{
					m_clientSideVertices[i].pos *= scale;
					UpdateAABB(m_clientSideVertices[i].pos);
				}
			}
		}
	}

	void Mesh::ConstructFaces()
	{
		size_t triCnt = m_clientSideIndices.size() / 3;
		m_faces.resize(triCnt);
		for (size_t i = 0; i < triCnt; i++)
		{
			if (m_clientSideIndices.empty())
			{
				for (size_t j = 0; j < 3; j++)
				{
					m_faces[i].vertices[j] = &m_clientSideVertices[i * 3 + j];
				}
			}
			else
			{
				for (size_t j = 0; j < 3; j++)
				{
					size_t indx = m_clientSideIndices[i * 3 + j];
					m_faces[i].vertices[j] = &m_clientSideVertices[indx];
				}
			}
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

		XmlFile file(m_file.c_str());
		XmlDocument doc;
		doc.parse<0>(file.data());

		XmlNode* node = doc.first_node("meshContainer");
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

			XmlNode* materialNode = node->first_node("material");
			String matFile = materialNode->first_attribute("name")->value();

			if (CheckFile(MaterialPath(matFile)))
			{
				mesh->m_material = GetMaterialManager()->Create(MaterialPath(matFile));
			}
			else
			{
				mesh->m_material = GetMaterialManager()->Create(MaterialPath("default.material"));
			}

			XmlNode* vertex = node->first_node("vertices");
			for (XmlNode* v = vertex->first_node("v"); v; v = v->next_sibling())
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

			XmlNode* faces = node->first_node("faces");
			for (XmlNode* i = faces->first_node("f"); i; i = i->next_sibling())
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

	int SkinMesh::GetVertexSize() const
	{
		return sizeof(SkinVertex);
	}

	bool SkinMesh::IsSkinned() const
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
