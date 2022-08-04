#include "Mesh.h"

#include <unordered_map>

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include "ToolKit.h"
#include "Material.h"
#include "Texture.h"
#include "Skeleton.h"
#include "Util.h"
#include "GL/glew.h"
#include "DebugNew.h"

namespace ToolKit
{

  Mesh::Mesh()
  {
    m_material = std::make_shared<Material>();
  }

  Mesh::Mesh(const String& file)
    : Mesh()
  {
    SetFile(file);
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
    if (!flushClientSideArray)
    {
      ConstructFaces();
    }

    m_material->Init(flushClientSideArray);

    for (MeshPtr mesh : m_subMeshes)
    {
      mesh->Init(flushClientSideArray);
    }

    m_initiated = true;
  }

  void Mesh::UnInit()
  {
    // If mesh class is only used to serialize/deserialize (nothing GPU related)
    //  then GPU buffers may not be initialized, so don't need to call these
    if (m_initiated)
    {
      GLuint buffers[2] = { m_vboIndexId, m_vboVertexId };
      glDeleteBuffers(2, buffers);
      glDeleteVertexArrays(1, &m_vaoId);
    }
    m_vboVertexId = 0;
    m_vboIndexId = 0;

    m_vaoId = 0;

    m_subMeshes.clear();


    m_initiated = false;
  }

  void Mesh::Load()
  {
    if (m_loaded)
    {
      return;
    }

    String path = GetFile();
    NormalizePath(path);
    XmlFile file = GetFileManager()->GetXmlFile(path);
    XmlDocument doc;
    doc.parse<0>(file.data());

    if (XmlNode* node = doc.first_node("meshContainer"))
    {
      DeSerialize(&doc, node);
      m_loaded = true;
    }
  }

  void Mesh::Save(bool onlyIfDirty)
  {
    // Force save if child is dirty.
    Resource::Save(!m_dirty && !m_material->m_dirty);
    m_material->Save(onlyIfDirty);
  }

  void Mesh::CopyTo(Resource* other)
  {
    Resource::CopyTo(other);
    Mesh* cpy = static_cast<Mesh*> (other);
    cpy->m_clientSideVertices = m_clientSideVertices;
    cpy->m_vertexCount = m_vertexCount;
    cpy->m_clientSideIndices = m_clientSideIndices;
    cpy->m_indexCount = m_indexCount;
    cpy->m_faces = m_faces;

    // Copy video memory.
    if (m_vertexCount > 0)
    {
      glGenVertexArrays(1, &cpy->m_vaoId);
      glBindVertexArray(cpy->m_vaoId);

      glGenBuffers(1, &cpy->m_vboVertexId);
      glBindBuffer(GL_COPY_WRITE_BUFFER, cpy->m_vboVertexId);
      glBindBuffer(GL_COPY_READ_BUFFER, m_vboVertexId);
      uint size = GetVertexSize() * m_vertexCount;
      glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_STATIC_DRAW);
      glCopyBufferSubData
      (
        GL_COPY_READ_BUFFER,
        GL_COPY_WRITE_BUFFER,
        0,
        0,
        size
      );
    }

    if (m_indexCount > 0)
    {
      glGenBuffers(1, &cpy->m_vboIndexId);
      glBindBuffer(GL_COPY_WRITE_BUFFER, cpy->m_vboIndexId);
      glBindBuffer(GL_COPY_READ_BUFFER, m_vboIndexId);
      uint size = sizeof(uint) * m_indexCount;
      glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_STATIC_DRAW);
      glCopyBufferSubData
      (
        GL_COPY_READ_BUFFER,
        GL_COPY_WRITE_BUFFER,
        0,
        0,
        size
      );
    }

    cpy->m_material = m_material->Copy<Material>();
    cpy->m_aabb = m_aabb;

    for (MeshPtr child : m_subMeshes)
    {
      MeshPtr ccpy = child->Copy<Mesh>();
      cpy->m_subMeshes.push_back(ccpy);
    }
  }

  int Mesh::GetVertexSize() const
  {
    return sizeof(Vertex);
  }

  bool Mesh::IsSkinned() const
  {
    return false;
  }

  void Mesh::CalculateAABB()
  {
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
        m_aabb.UpdateBoundary(v.pos);
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

  void Mesh::GetAllMeshes(MeshRawCPtrArray& meshes) const
  {
    meshes.push_back(this);
    for (size_t i = 0; i < m_subMeshes.size(); i++)
    {
      m_subMeshes[i]->GetAllMeshes(meshes);
    }
  }

  template<typename T>
  void ConstructFacesT(T* mesh)
  {
    size_t triCnt = mesh->m_clientSideIndices.size() / 3;
    if (mesh->m_clientSideIndices.empty())
    {
      triCnt = mesh->m_clientSideVertices.size() / 3;
    }

    mesh->m_faces.resize(triCnt);
    for (size_t i = 0; i < triCnt; i++)
    {
      if (mesh->m_clientSideIndices.empty())
      {
        for (size_t j = 0; j < 3; j++)
        {
          mesh->m_faces[i].vertices[j] = &mesh->m_clientSideVertices[i * 3 + j];
        }
      }
      else
      {
        for (size_t j = 0; j < 3; j++)
        {
          size_t indx = mesh->m_clientSideIndices[i * 3 + j];
          mesh->m_faces[i].vertices[j] = &mesh->m_clientSideVertices[indx];
        }
      }
    }
  }

  void Mesh::ConstructFaces()
  {
    if (IsSkinned())
    {
      ConstructFacesT(reinterpret_cast<SkinMesh*>(this));
    }
    else
    {
      ConstructFacesT(this);
    }
  }

  void Mesh::ApplyTransform(const Mat4& transform)
  {
    Mat4 its = glm::inverseTranspose(transform);
    for (Vertex& v : m_clientSideVertices)
    {
      v.pos = transform * Vec4(v.pos, 1.0f);
      v.norm = glm::normalize(its * Vec4(v.norm, 1.0f));
      v.btan = glm::normalize(its * Vec4(v.btan, 1.0f));
    }
  }

  void Mesh::SetMaterial(MaterialPtr material)
  {
    m_material = material;
    m_material->Init(false);
    m_dirty = true;
    Save(true);
    m_dirty = false;
  }

  template<typename T>
  void writeMesh(XmlDocument* doc, XmlNode* parent, const T* mesh)
  {
    XmlNode* meshNode;
    if constexpr (std::is_same<T, ToolKit::SkinMesh>::value)
    {
      meshNode = CreateXmlNode(doc, "skinMesh", parent);
    }
    else
    {
      meshNode = CreateXmlNode(doc, "mesh", parent);
    }

    WriteMaterial(meshNode, doc, mesh->m_material->GetSerializeFile());

    // Write Skeleton file reference
    if constexpr (std::is_same<T, ToolKit::SkinMesh>::value)
    {
      mesh->m_skeleton->SerializeRef(doc, meshNode);
    }


    XmlNode* vertices = CreateXmlNode(doc, "vertices", meshNode);

    // Serialize vertex
    for (const auto& v : mesh->m_clientSideVertices)
    {
      XmlNode* vNod = CreateXmlNode(doc, "v", vertices);

      XmlNode* p = CreateXmlNode(doc, "p", vNod);
      WriteVec(p, doc, v.pos);

      XmlNode* n = CreateXmlNode(doc, "n", vNod);
      WriteVec(n, doc, v.norm);

      XmlNode* t = CreateXmlNode(doc, "t", vNod);
      WriteVec(t, doc, v.tex);

      XmlNode* bt = CreateXmlNode(doc, "bt", vNod);
      WriteVec(bt, doc, v.btan);
      if constexpr (std::is_same<T, ToolKit::SkinMesh>::value)
      {
        XmlNode* b = CreateXmlNode(doc, "b", vNod);
        WriteVec(b, doc, v.bones);

        XmlNode* w = CreateXmlNode(doc, "w", vNod);
        WriteVec(w, doc, v.weights);
      }
    }

    // Serialize faces
    XmlNode* faces = CreateXmlNode(doc, "faces", meshNode);

    for (size_t i = 0; i < mesh->m_clientSideIndices.size() / 3; i++)
    {
      XmlNode* f = CreateXmlNode(doc, "f", faces);

      WriteAttr
      (
        f,
        doc,
        "x",
        std::to_string(mesh->m_clientSideIndices[i * 3])
      );
      WriteAttr
      (
        f,
        doc,
        "y",
        std::to_string(mesh->m_clientSideIndices[i * 3 + 1])
      );
      WriteAttr
      (
        f,
        doc,
        "z",
        std::to_string(mesh->m_clientSideIndices[i * 3 + 2])
      );
    }
  };

  void Mesh::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* container = CreateXmlNode(doc, "meshContainer", parent);


    // This approach will flatten the mesh on a single sibling level.
    // To keep the depth hierarch, recursive save is needed.
    MeshRawCPtrArray cMeshes;
    GetAllMeshes(cMeshes);
    for (const Mesh* m : cMeshes)
    {
      if (m->IsSkinned())
      {
        writeMesh(doc, container, static_cast<const SkinMesh*>(m));
      }
      else
      {
        writeMesh(doc, container, static_cast<const Mesh*>(m));
      }
    }
  }

  void Mesh::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    m_aabb = BoundingBox();

    Mesh* mesh = this;
    XmlNode* node = parent;
    for
    (
      node = node->first_node("mesh");
      node;
      node = node->next_sibling("mesh")
    )
    {
      if (mesh == nullptr)
      {
        mesh = new Mesh();
        m_subMeshes.push_back(MeshPtr(mesh));
      }

      mesh->m_material = ReadMaterial(node);

      XmlNode* vertex = node->first_node("vertices");
      for (XmlNode* v = vertex->first_node("v"); v; v = v->next_sibling())
      {
        Vertex vd;
        ReadVec(v->first_node("p"), vd.pos);
        m_aabb.UpdateBoundary(vd.pos);

        ReadVec(v->first_node("n"), vd.norm);
        ReadVec(v->first_node("t"), vd.tex);
        ReadVec(v->first_node("bt"), vd.btan);
        mesh->m_clientSideVertices.push_back(vd);
      }

      XmlNode* faces = node->first_node("faces");
      for (XmlNode* i = faces->first_node("f"); i; i = i->next_sibling())
      {
        glm::ivec3 indices;
        ReadVec(i, indices);
        mesh->m_clientSideIndices.push_back(indices.x);
        mesh->m_clientSideIndices.push_back(indices.y);
        mesh->m_clientSideIndices.push_back(indices.z);
      }

      mesh->m_loaded = true;
      mesh->m_vertexCount = static_cast<int>(mesh->m_clientSideVertices.size());
      mesh->m_indexCount = static_cast<int>(mesh->m_clientSideIndices.size());
      mesh = nullptr;
    }
  }

  void Mesh::InitVertices(bool flush)
  {
    glDeleteBuffers(1, &m_vboVertexId);
    glDeleteVertexArrays(1, &m_vaoId);

    if (!m_clientSideVertices.empty())
    {
      glGenVertexArrays(1, &m_vaoId);
      glBindVertexArray(m_vaoId);

      glGenBuffers(1, &m_vboVertexId);
      glBindBuffer(GL_ARRAY_BUFFER, m_vboVertexId);
      glBufferData
      (
        GL_ARRAY_BUFFER,
        GetVertexSize() * m_clientSideVertices.size(),
        m_clientSideVertices.data(), GL_STATIC_DRAW
      );
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
      glBufferData
      (
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(uint) * m_clientSideIndices.size(),
        m_clientSideIndices.data(),
        GL_STATIC_DRAW
      );
      m_indexCount = (uint)m_clientSideIndices.size();
    }

    m_indexCount = (uint)m_clientSideIndices.size();
    if (flush)
    {
      m_clientSideIndices.clear();
    }
  }

  SkinMesh::SkinMesh()
    : Mesh()
  {
  }

  SkinMesh::SkinMesh(const String& file)
    : Mesh()
  {
    SetFile(file);

    String skelFile = file.substr(0, file.find_last_of("."));
    skelFile += ".skeleton";

    m_skeleton = GetSkeletonManager()->Create<Skeleton>(skelFile);
  }

  SkinMesh::~SkinMesh()
  {
    UnInit();
  }

  void SkinMesh::Init(bool flushClientSideArray)
  {
    if (m_skeleton == nullptr)
    {
      return;
    }
    m_skeleton->Init(flushClientSideArray);
    Mesh::Init(flushClientSideArray);
  }

  void SkinMesh::UnInit()
  {
    m_initiated = false;
  }

  void SkinMesh::Load()
  {
    if (m_loaded)
    {
      return;
    }

    // If skeleton is specified, load it
    // While reading from a file, it's probably not loaded
    // So Deserialize will also try to load it
    if (m_skeleton)
    {
      m_skeleton->Load();
      assert(m_skeleton->m_loaded);
      if (!m_skeleton->m_loaded)
      {
        return;
      }
    }



    String path = GetFile();
    NormalizePath(path);
    XmlFile file = GetFileManager()->GetXmlFile(path);
    XmlDocument doc;
    doc.parse<0>(file.data());

    if (XmlNode* node = doc.first_node("meshContainer"))
    {
      DeSerialize(&doc, node);
      m_loaded = true;
    }
  }
  void SkinMesh::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    m_aabb = BoundingBox();

    SkinMesh* mesh = this;
    XmlNode* node = parent;
    for
      (
        node = node->first_node("skinMesh");
        node;
        node = node->next_sibling("skinMesh")
      )
    {
      if (mesh == nullptr)
      {
        mesh = new SkinMesh();
        m_subMeshes.push_back(MeshPtr(mesh));
      }

      mesh->m_material = ReadMaterial(node);

      String path = Skeleton::DeserializeRef(node);
      if (path.length() == 0)
      {
        assert(0 && "SkinMesh has no skeleton!");
      }
      NormalizePath(path);
      String skelFile = SkeletonPath(path);
      mesh->m_skeleton = GetSkeletonManager()->Create<Skeleton>(skelFile);


      XmlNode* vertex = node->first_node("vertices");
      for (XmlNode* v = vertex->first_node("v"); v; v = v->next_sibling())
      {
        SkinVertex vd;
        ReadVec(v->first_node("p"), vd.pos);
        m_aabb.UpdateBoundary(vd.pos);

        ReadVec(v->first_node("n"), vd.norm);
        ReadVec(v->first_node("t"), vd.tex);
        ReadVec(v->first_node("bt"), vd.btan);
        ReadVec(v->first_node("b"), vd.bones);
        ReadVec(v->first_node("w"), vd.weights);
        mesh->m_clientSideVertices.push_back(vd);
      }

      XmlNode* faces = node->first_node("faces");
      for (XmlNode* i = faces->first_node("f"); i; i = i->next_sibling())
      {
        glm::ivec3 indices;
        ReadVec(i, indices);
        mesh->m_clientSideIndices.push_back(indices.x);
        mesh->m_clientSideIndices.push_back(indices.y);
        mesh->m_clientSideIndices.push_back(indices.z);
      }

      mesh->m_loaded = true;
      mesh->m_vertexCount = static_cast<int>(mesh->m_clientSideVertices.size());
      mesh->m_indexCount = static_cast<int>(mesh->m_clientSideIndices.size());
      mesh = nullptr;
    }
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
      glBufferData
      (
        GL_ARRAY_BUFFER,
        GetVertexSize() * m_clientSideVertices.size(),
        m_clientSideVertices.data(),
        GL_STATIC_DRAW
      );
      m_vertexCount = (uint)m_clientSideVertices.size();
    }

    if (flush)
    {
      m_clientSideVertices.clear();
    }
  }


  MeshManager::MeshManager()
  {
    m_type = ResourceType::Mesh;
  }

  MeshManager::~MeshManager()
  {
  }

  bool MeshManager::CanStore(ResourceType t)
  {
    if
    (
      t == ResourceType::Mesh
      || t == ResourceType::SkinMesh
    )
    {
      return true;
    }

    return false;
  }

  ResourcePtr MeshManager::CreateLocal(ResourceType type)
  {
    Mesh* res = nullptr;
    switch (type)
    {
    case ResourceType::Mesh:
      res = new Mesh();
      break;
    case ResourceType::SkinMesh:
      res = new SkinMesh();
      break;
    default:
      assert(false);
      break;
    }
    return ResourcePtr(res);
  }

  String MeshManager::GetDefaultResource(ResourceType type)
  {
    return MeshPath("Suzanne.mesh", true);
  }
}  // namespace ToolKit
