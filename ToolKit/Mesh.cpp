/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Mesh.h"

#include "Common/base64.h"
#include "FileManager.h"
#include "Material.h"
#include "MathUtil.h"
#include "RHI.h"
#include "ResourceManager.h"
#include "Skeleton.h"
#include "TKOpenGL.h"
#include "TKStats.h"
#include "Texture.h"
#include "Threads.h"
#include "ToolKit.h"
#include "Util.h"

#include "DebugNew.h"

static constexpr bool SERIALIZE_MESH_AS_BINARY = true;

namespace ToolKit
{

#define BUFFER_OFFSET(idx) (static_cast<char*>(0) + (idx))

  void SetVertexLayout(VertexLayout layout)
  {
    if (layout == VertexLayout::None)
    {
      for (int i = 0; i < 6; i++)
      {
        glDisableVertexAttribArray(i);
      }
    }

    if (layout == VertexLayout::Mesh)
    {
      GLuint offset = 0;
      glEnableVertexAttribArray(0); // Vertex
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
      offset += 3 * sizeof(float);

      glEnableVertexAttribArray(1); // Normal
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offset));
      offset += 3 * sizeof(float);

      glEnableVertexAttribArray(2); // Texture
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offset));
      offset += 2 * sizeof(float);

      glEnableVertexAttribArray(3); // BiTangent
      glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offset));
    }

    if (layout == VertexLayout::SkinMesh)
    {
      GLuint offset = 0;
      glEnableVertexAttribArray(0); // Vertex
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), 0);
      offset += 3 * sizeof(float);

      glEnableVertexAttribArray(1); // Normal
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), BUFFER_OFFSET(offset));
      offset += 3 * sizeof(float);

      glEnableVertexAttribArray(2); // Texture
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), BUFFER_OFFSET(offset));
      offset += 2 * sizeof(float);

      glEnableVertexAttribArray(3); // BiTangent
      glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), BUFFER_OFFSET(offset));
      offset += 3 * sizeof(float);

      glEnableVertexAttribArray(4); // Bones
      glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), BUFFER_OFFSET(offset));
      offset += 4 * sizeof(float);

      glEnableVertexAttribArray(5); // Weights
      glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), BUFFER_OFFSET(offset));
    }
  }

  // Mesh
  //////////////////////////////////////////////////////////////////////////

  TKDefineClass(Mesh, Resource);

  Mesh::Mesh()
  {
    m_material     = GetMaterialManager()->GetCopyOfDefaultMaterial(false);
    m_vertexLayout = VertexLayout::Mesh;
  }

  Mesh::Mesh(const String& file) : Mesh() { SetFile(file); }

  Mesh::~Mesh() { UnInit(); }

  void Mesh::Init(bool flushClientSideArray)
  {
    if (m_initiated)
    {
      return;
    }

    InitVertices(flushClientSideArray);
    SetVertexLayout(m_vertexLayout);
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
      if (m_vboVertexId)
      {
        RemoveVRAMUsageInBytes(GetVertexSize() * m_vertexCount);
      }

      if (m_vboIndexId)
      {
        RemoveVRAMUsageInBytes(sizeof(uint) * m_indexCount);
      }

      GLuint buffers[2] = {m_vboIndexId, m_vboVertexId};
      glDeleteBuffers(2, buffers);
      glDeleteVertexArrays(1, &m_vaoId);
      RHI::BindVertexArray(0); // Of the deleted vao is set, remove it from RHI cache
    }
    m_vboVertexId = 0;
    m_vboIndexId  = 0;

    m_vaoId       = 0;

    m_subMeshes.clear();

    m_initiated = false;
  }

  void Mesh::Load()
  {
    if (!m_loaded)
    {
      ParseDocument("meshContainer");
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
    Mesh* cpy                 = static_cast<Mesh*>(other);
    cpy->m_clientSideVertices = m_clientSideVertices;
    cpy->m_vertexCount        = m_vertexCount;
    cpy->m_clientSideIndices  = m_clientSideIndices;
    cpy->m_indexCount         = m_indexCount;
    cpy->m_faces              = m_faces;

    // Copy video memory.
    if (m_vertexCount > 0)
    {
      glGenVertexArrays(1, &cpy->m_vaoId);
      RHI::BindVertexArray(cpy->m_vaoId);

      glGenBuffers(1, &cpy->m_vboVertexId);
      glBindBuffer(GL_COPY_WRITE_BUFFER, cpy->m_vboVertexId);
      glBindBuffer(GL_COPY_READ_BUFFER, m_vboVertexId);
      uint size = GetVertexSize() * m_vertexCount;
      glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_STATIC_DRAW);
      glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);

      AddVRAMUsageInBytes(size);
    }

    if (m_indexCount > 0)
    {
      assert(m_vertexCount > 0 && "Mesh has no vertex but has indices.");

      RHI::BindVertexArray(cpy->m_vaoId);

      glGenBuffers(1, &cpy->m_vboIndexId);
      glBindBuffer(GL_COPY_WRITE_BUFFER, cpy->m_vboIndexId);
      glBindBuffer(GL_COPY_READ_BUFFER, m_vboIndexId);
      uint size = sizeof(uint) * m_indexCount;
      glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_STATIC_DRAW);
      glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);

      AddVRAMUsageInBytes(size);
    }

    cpy->m_material = GetMaterialManager()->Copy<Material>(m_material);
    cpy->m_aabb     = m_aabb;

    for (MeshPtr child : m_subMeshes)
    {
      MeshPtr childCopy = GetMeshManager()->Copy<Mesh>(child);
      cpy->m_subMeshes.push_back(childCopy);
    }
  }

  int Mesh::GetVertexSize() const { return sizeof(Vertex); }

  uint Mesh::GetVertexCount() const { return (uint) m_clientSideVertices.size(); }

  bool Mesh::IsSkinned() const { return false; }

  void Mesh::CalculateAABB()
  {
    // Construct aabb of all submeshes.
    MeshRawPtrArray meshes;
    GetAllMeshes(meshes);

    BoundingBox aabb;
    for (Mesh* mesh : meshes)
    {
      for (size_t i = 0; i < mesh->m_clientSideVertices.size(); i++)
      {
        Vertex& v = mesh->m_clientSideVertices[i];
        aabb.UpdateBoundary(v.pos);
      }
    }
    m_aabb = aabb;
  }

  void GetAllMeshHelper(const Mesh* mesh, MeshRawPtrArray& meshes)
  {
    if (mesh == nullptr)
    {
      return;
    }

    meshes.push_back(const_cast<Mesh*>(mesh));

    for (MeshPtr subMesh : mesh->m_subMeshes)
    {
      GetAllMeshHelper(subMesh.get(), meshes);
    }
  }

  void Mesh::GetAllMeshes(MeshRawPtrArray& meshes) const { GetAllMeshHelper(this, meshes); }

  template <typename T>
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
          size_t indx                  = mesh->m_clientSideIndices[i * 3 + j];
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
      v.pos  = transform * Vec4(v.pos, 1.0f);
      v.norm = glm::normalize(its * Vec4(v.norm, 1.0f));
      v.btan = glm::normalize(its * Vec4(v.btan, 1.0f));
    }
  }

  void Mesh::SetMaterial(MaterialPtr material)
  {
    m_material = material;
    m_material->Init();
    m_dirty = true;
  }

  template <typename T>
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
    if constexpr (SERIALIZE_MESH_AS_BINARY)
    {
      size_t vertexBufferDataSize = mesh->m_clientSideVertices.size() * sizeof(mesh->m_clientSideVertices[0]);
      WriteAttr(vertices, doc, "VertexCount", std::to_string(mesh->m_clientSideVertices.size()));
      char* b64Data = new char[vertexBufferDataSize * 2];
      bintob64(b64Data, mesh->m_clientSideVertices.data(), vertexBufferDataSize);
      XmlNode* base64XML = CreateXmlNode(doc, "Base64", vertices);
      base64XML->value(doc->allocate_string(b64Data));
      SafeDelArray(b64Data);
    }
    else
    {
      for (const auto& v : mesh->m_clientSideVertices)
      {
        XmlNode* vNod = CreateXmlNode(doc, "v", vertices);

        XmlNode* p    = CreateXmlNode(doc, "p", vNod);
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
    }

    // Serialize faces
    XmlNode* faces = CreateXmlNode(doc, "faces", meshNode);
    if constexpr (SERIALIZE_MESH_AS_BINARY)
    {
      size_t facesBufferDataSize = mesh->m_clientSideIndices.size() * sizeof(mesh->m_clientSideIndices[0]);
      WriteAttr(faces, doc, "FaceCount", std::to_string(mesh->m_clientSideIndices.size()));
      char* b64Data = new char[facesBufferDataSize * 2];
      bintob64(b64Data, mesh->m_clientSideIndices.data(), facesBufferDataSize);
      XmlNode* base64XML = CreateXmlNode(doc, "Base64", faces);
      base64XML->value(doc->allocate_string(b64Data));
      SafeDelArray(b64Data);
    }
    else
    {
      for (size_t i = 0; i < mesh->m_clientSideIndices.size() / 3; i++)
      {
        XmlNode* f = CreateXmlNode(doc, "f", faces);

        WriteAttr(f, doc, "x", std::to_string(mesh->m_clientSideIndices[i * 3]));
        WriteAttr(f, doc, "y", std::to_string(mesh->m_clientSideIndices[i * 3 + 1]));
        WriteAttr(f, doc, "z", std::to_string(mesh->m_clientSideIndices[i * 3 + 2]));
      }
    }
  };

  template <typename T>
  void LoadMesh(XmlDocument* doc, XmlNode* parent, T* mainMesh)
  {
    mainMesh->m_aabb = BoundingBox();

    T* mesh          = mainMesh;
    XmlNode* node    = parent;

    String typeString;
    if constexpr (std::is_same<T, Mesh>())
    {
      typeString = "mesh";
    }
    else
    {
      typeString = "skinMesh";
    }
    for (node = node->first_node(typeString.c_str()); node; node = node->next_sibling(typeString.c_str()))
    {
      if (mesh == nullptr)
      {
        std::shared_ptr<T> meshPtr = MakeNewPtr<T>();
        mesh                       = meshPtr.get();
        mainMesh->m_subMeshes.push_back(meshPtr);
      }

      mesh->m_material = ReadMaterial(node);

      if constexpr (std::is_same<T, SkinMesh>())
      {
        String path = Skeleton::DeserializeRef(node);
        if (path.length() == 0)
        {
          assert(0 && "SkinMesh has no skeleton!");
        }

        NormalizePath(path);
        String skelFile  = SkeletonPath(path);
        mesh->m_skeleton = GetSkeletonManager()->Create<Skeleton>(skelFile);
      }

      XmlNode* vertex = node->first_node("vertices");
      // Vertex Buffer stored as binary
      if (XmlAttribute* dataSizeAttr = vertex->first_attribute("VertexCount"))
      {
        uint vertexCount = 0;
        ReadAttr(vertex, "VertexCount", vertexCount);
        mesh->m_clientSideVertices.resize(vertexCount);
        XmlNode* b64Node = vertex->first_node("Base64");
        b64tobin(mesh->m_clientSideVertices.data(), b64Node->value());

        if constexpr (std::is_same<T, Mesh>())
        {
          mesh->CalculateAABB();
        }
      }
      else
      {
        for (XmlNode* v = vertex->first_node("v"); v; v = v->next_sibling())
        {
          SkinVertex vd;
          ReadVec(v->first_node("p"), vd.pos);
          mainMesh->m_aabb.UpdateBoundary(vd.pos);

          ReadVec(v->first_node("n"), vd.norm);
          ReadVec(v->first_node("t"), vd.tex);
          ReadVec(v->first_node("bt"), vd.btan);
          if constexpr (std::is_same<T, SkinMesh>())
          {
            ReadVec(v->first_node("b"), vd.bones);
            ReadVec(v->first_node("w"), vd.weights);
          }
          mesh->m_clientSideVertices.push_back(vd);
        }
      }

      XmlNode* faces = node->first_node("faces");

      if (XmlAttribute* faceCountAttr = faces->first_attribute("FaceCount"))
      {
        uint faceCount = 0;
        ReadAttr(faces, "FaceCount", faceCount);
        mesh->m_clientSideIndices.resize(faceCount);
        XmlNode* b64Node = faces->first_node("Base64");
        b64tobin(mesh->m_clientSideIndices.data(), b64Node->value());
      }
      else
      {
        for (XmlNode* i = faces->first_node("f"); i; i = i->next_sibling())
        {
          glm::ivec3 indices;
          ReadVec(i, indices);
          mesh->m_clientSideIndices.push_back(indices.x);
          mesh->m_clientSideIndices.push_back(indices.y);
          mesh->m_clientSideIndices.push_back(indices.z);
        }
      }

      mesh->m_loaded      = true;
      mesh->m_vertexCount = (int) (mesh->m_clientSideVertices.size());
      mesh->m_indexCount  = (int) (mesh->m_clientSideIndices.size());
      mesh                = nullptr;
    }
  }

  XmlNode* Mesh::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* container = CreateXmlNode(doc, "meshContainer", parent);

    // This approach will flatten the mesh on a single sibling level.
    // To keep the depth hierarchy, recursive save is needed.
    MeshRawPtrArray cMeshes;
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

    return container;
  }

  XmlNode* Mesh::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    LoadMesh(info.Document, parent, this);
    return nullptr;
  }

  void Mesh::InitVertices(bool flush)
  {
    if (m_vboVertexId != 0)
    {
      RemoveVRAMUsageInBytes(GetVertexSize() * m_vertexCount);
    }

    glDeleteBuffers(1, &m_vboVertexId);
    glDeleteVertexArrays(1, &m_vaoId);
    RHI::BindVertexArray(0); // Of the deleted vao is set, remove it from RHI cache

    if (!m_clientSideVertices.empty())
    {
      glGenVertexArrays(1, &m_vaoId);
      RHI::BindVertexArray(m_vaoId);

      glGenBuffers(1, &m_vboVertexId);
      glBindBuffer(GL_ARRAY_BUFFER, m_vboVertexId);

      glBufferData(GL_ARRAY_BUFFER,
                   GetVertexSize() * m_clientSideVertices.size(),
                   m_clientSideVertices.data(),
                   GL_STATIC_DRAW);
    }

    m_vertexCount = (uint) m_clientSideVertices.size();
    AddVRAMUsageInBytes(GetVertexSize() * m_vertexCount);

    if (flush)
    {
      m_clientSideVertices.clear();
    }
  }

  void Mesh::InitIndices(bool flush)
  {
    if (m_vboIndexId != 0)
    {
      RemoveVRAMUsageInBytes(sizeof(uint) * m_indexCount);
    }

    glDeleteBuffers(1, &m_vboIndexId);

    if (!m_clientSideIndices.empty())
    {
      assert(m_vaoId != 0 && "Mesh has not yet created vertex array object!");

      RHI::BindVertexArray(m_vaoId);

      glGenBuffers(1, &m_vboIndexId);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboIndexId);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   sizeof(uint) * m_clientSideIndices.size(),
                   m_clientSideIndices.data(),
                   GL_STATIC_DRAW);
      m_indexCount = (uint) m_clientSideIndices.size();

      AddVRAMUsageInBytes(sizeof(uint) * m_clientSideIndices.size());
    }

    m_indexCount = (uint) m_clientSideIndices.size();
    if (flush)
    {
      m_clientSideIndices.clear();
    }
  }

  // SkinMesh
  //////////////////////////////////////////////////////////////////////////

  TKDefineClass(SkinMesh, Mesh);

  SkinMesh::SkinMesh() : Mesh() { m_vertexLayout = VertexLayout::SkinMesh; }

  SkinMesh::SkinMesh(const String& file) : SkinMesh()
  {
    SetFile(file);

    String skelFile  = file.substr(0, file.find_last_of("."));
    skelFile        += ".skeleton";

    m_skeleton       = GetSkeletonManager()->Create<Skeleton>(skelFile);
  }

  SkinMesh::~SkinMesh() { UnInit(); }

  void SkinMesh::Init(bool flushClientSideArray)
  {
    if (m_skeleton == nullptr)
    {
      return;
    }
    // Don't flush, otherwise CPU skinning won't work. So neither
    // CalculateBoundary() nor RayMeshIntersection() will work as expected
    m_skeleton->Init(false);
    Mesh::Init(false);
  }

  void SkinMesh::UnInit() { Mesh::UnInit(); }

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

    ParseDocument("meshContainer");
    m_loaded = true;
  }

  XmlNode* SkinMesh::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    LoadMesh(info.Document, parent, this);
    return nullptr;
  }

  BoundingBox SkinMesh::CalculateAABB(const Skeleton* skel, DynamicBoneMapPtr boneMap)
  {
    if (m_bindPoseAABBCalculated)
    {
      return m_bindPoseAABB;
    }

    BoundingBox finalAABB;
    MeshRawPtrArray meshes;
    GetAllMeshes(meshes);

    std::vector<BoundingBox> AABBs(meshes.size());
    UIntArray indexes(meshes.size());
    for (uint i = 0; i < meshes.size(); i++)
    {
      indexes[i] = i;
    }

    std::for_each(indexes.begin(),
                  indexes.end(),
                  [skel, boneMap, &AABBs, &meshes](uint index)
                  {
                    SkinMesh* m = (SkinMesh*) meshes[index];
                    if (m->m_clientSideVertices.empty())
                    {
                      return;
                    }
                    BoundingBox& meshAABB = AABBs[index];
                    std::mutex meshAABBLocker;

                    std::for_each(TKExecBy(WorkerManager::FramePool),
                                  m->m_clientSideVertices.begin(),
                                  m->m_clientSideVertices.end(),
                                  [skel, boneMap, &meshAABBLocker, &meshAABB](SkinVertex& v)
                                  {
                                    Vec3 skinnedPos = CPUSkinning(&v, skel, boneMap, false);
                                    std::lock_guard<std::mutex> guard(meshAABBLocker);
                                    meshAABB.UpdateBoundary(skinnedPos);
                                  });
                  });

    for (BoundingBox& aabb : AABBs)
    {
      finalAABB.UpdateBoundary(aabb.max);
      finalAABB.UpdateBoundary(aabb.min);
    }

    m_bindPoseAABBCalculated = true;
    m_bindPoseAABB           = finalAABB;

    return finalAABB;
  }

  uint SkinMesh::GetVertexCount() const { return (uint) m_clientSideVertices.size(); }

  int SkinMesh::GetVertexSize() const { return sizeof(SkinVertex); }

  bool SkinMesh::IsSkinned() const { return true; }

  void SkinMesh::InitVertices(bool flush)
  {
    glDeleteBuffers(1, &m_vboIndexId);
    glDeleteVertexArrays(1, &m_vaoId);
    RHI::BindVertexArray(0); // Of the deleted vao is set, remove it from RHI cache

    if (!m_clientSideVertices.empty())
    {
      glGenVertexArrays(1, &m_vaoId);
      RHI::BindVertexArray(m_vaoId);

      glGenBuffers(1, &m_vboVertexId);
      glBindBuffer(GL_ARRAY_BUFFER, m_vboVertexId);
      glBufferData(GL_ARRAY_BUFFER,
                   GetVertexSize() * m_clientSideVertices.size(),
                   m_clientSideVertices.data(),
                   GL_STATIC_DRAW);
      m_vertexCount = (uint) m_clientSideVertices.size();

      AddVRAMUsageInBytes(GetVertexSize() * m_clientSideVertices.size());
    }

    if (flush)
    {
      m_clientSideVertices.clear();
    }
  }

  void SkinMesh::CopyTo(Resource* other)
  {
    Mesh::CopyTo(other);
    SkinMesh* cpy   = static_cast<SkinMesh*>(other);
    cpy->m_skeleton = GetSkeletonManager()->Copy<Skeleton>(m_skeleton);
    cpy->m_skeleton->Init();
  }

  MeshManager::MeshManager() { m_baseType = Mesh::StaticClass(); }

  MeshManager::~MeshManager() {}

  bool MeshManager::CanStore(ClassMeta* Class) { return Class->IsSublcassOf(Mesh::StaticClass()); }

  String MeshManager::GetDefaultResource(ClassMeta* Class) { return MeshPath("Suzanne.mesh", true); }

} // namespace ToolKit
