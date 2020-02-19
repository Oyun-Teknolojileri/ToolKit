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

ToolKit::Mesh::Mesh()
{
  m_material = std::shared_ptr<Material>(new Material());
}

ToolKit::Mesh::Mesh(std::string file)
{
  m_file = file;
  m_material = std::shared_ptr<Material>(new Material());
}

ToolKit::Mesh::~Mesh()
{
	UnInit();
}

void ToolKit::Mesh::Init(bool flushClientSideArray)
{
  if (m_initiated)
  {
    return;
  }

  InitVertices(flushClientSideArray);
  InitIndices(flushClientSideArray);
  m_material->Init();

  for (std::shared_ptr<Mesh> mesh : m_subMeshes)
  {
    mesh->Init(flushClientSideArray);
  }

  m_initiated = true;
}

void ToolKit::Mesh::UnInit()
{
	GLuint buffers[2] = { m_vboIndexId, m_vboVertexId };
	glDeleteBuffers(2, buffers);

  for (std::shared_ptr<Mesh> subMesh : m_subMeshes)
  {
		subMesh = nullptr;
  }

	m_initiated = false;
}

void ToolKit::Mesh::Load()
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

	m_AABoundingBox.min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	m_AABoundingBox.max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

  Mesh* mesh = this;
  for (node = node->first_node("mesh"); node; node = node->next_sibling("mesh"))
  {
    if (mesh == nullptr)
    {
      mesh = new Mesh();
      m_subMeshes.push_back(std::shared_ptr<Mesh>(mesh));
    }

    rapidxml::xml_node<>* materialNode = node->first_node("material");
    std::string matFile = materialNode->first_attribute("name")->value();

    if (CheckFile(MaterialPath(matFile)))
    {
      mesh->m_material = Main::GetInstance()->m_materialManager.Create(MaterialPath(matFile));
    }
    else
    {
      mesh->m_material = Main::GetInstance()->m_materialManager.Create(MaterialPath("default.material"));
    }

    rapidxml::xml_node<>* vertex = node->first_node("vertices");
    for (rapidxml::xml_node<>* v = vertex->first_node("v"); v; v = v->next_sibling())
    {
      Vertex vd;
      ExtractXYZFromNode(v->first_node("p"), vd.pos);
			UpdateAABBMax(vd.pos);
			UpdateAABBMin(vd.pos);

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

int ToolKit::Mesh::GetVertexSize()
{
  return sizeof(Vertex);
}

bool ToolKit::Mesh::IsSkinned()
{
  return false;
}

void ToolKit::Mesh::CalculateAABoundingBox()
{
	if (m_clientSideVertices.empty())
	{
		return;
	}

	m_AABoundingBox.min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	m_AABoundingBox.max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	std::vector<Mesh*> meshes;
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
			Vertex& v = m_clientSideVertices[j];
			UpdateAABBMax(v.pos);
			UpdateAABBMin(v.pos);
		}
	}
}

void ToolKit::Mesh::GetAllMeshes(std::vector<Mesh*>& meshes)
{
	meshes.push_back(this);
	for (size_t i = 0; i < m_subMeshes.size(); i++)
	{
		m_subMeshes[i]->GetAllMeshes(meshes);
	}
}

void ToolKit::Mesh::InitVertices(bool flush)
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

void ToolKit::Mesh::InitIndices(bool flush)
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

void ToolKit::Mesh::UpdateAABBMax(const glm::vec3& v)
{
	if (m_AABoundingBox.max.x < v.x)
	{
		m_AABoundingBox.max.x = v.x;
	}

	if (m_AABoundingBox.max.y < v.y)
	{
		m_AABoundingBox.max.y = v.y;
	}

	if (m_AABoundingBox.max.z < v.z)
	{
		m_AABoundingBox.max.z = v.z;
	}
}

void ToolKit::Mesh::UpdateAABBMin(const glm::vec3& v)
{
	if (m_AABoundingBox.min.x > v.x)
	{
		m_AABoundingBox.min.x = v.x;
	}

	if (m_AABoundingBox.min.y > v.y)
	{
		m_AABoundingBox.min.y = v.y;
	}

	if (m_AABoundingBox.min.z > v.z)
	{
		m_AABoundingBox.min.z = v.z;
	}
}

ToolKit::SkinMesh::SkinMesh()
{
  m_skeleton = new Skeleton();
}

ToolKit::SkinMesh::SkinMesh(std::string file)
  : Mesh(file)
{
  std::string skelFile = file.substr(0, file.find_last_of("."));
  skelFile += ".skeleton";

  m_skeleton = new Skeleton(skelFile);
}

ToolKit::SkinMesh::~SkinMesh()
{
	UnInit();
}

void ToolKit::SkinMesh::Init(bool flushClientSideArray)
{
  m_skeleton->Init(flushClientSideArray);
  Mesh::Init(flushClientSideArray);
}

void ToolKit::SkinMesh::UnInit()
{
	SafeDel(m_skeleton);
	m_initiated = false;
}

void ToolKit::SkinMesh::Load()
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
      m_subMeshes.push_back(std::shared_ptr<Mesh>(mesh));
    }

    rapidxml::xml_node<>* materialNode = node->first_node("material");
    std::string matFile = materialNode->first_attribute("name")->value();

    if (CheckFile(MaterialPath(matFile)))
    {
      mesh->m_material = Main::GetInstance()->m_materialManager.Create(MaterialPath(matFile));
    }
    else
    {
      mesh->m_material = Main::GetInstance()->m_materialManager.Create(MaterialPath("default.material"));
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

int ToolKit::SkinMesh::GetVertexSize()
{
  return sizeof(SkinVertex);
}

bool ToolKit::SkinMesh::IsSkinned()
{
  return true;
}

void ToolKit::SkinMesh::InitVertices(bool flush)
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
