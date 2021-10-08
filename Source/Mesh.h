#pragma once

#include "Types.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "MathUtil.h"
#include "GL/glew.h"
#include <memory>

namespace ToolKit
{

  class Vertex
  {
  public:
    Vec3 pos;
    Vec3 norm;
    Vec2 tex;
    Vec3 btan;
  };

  class Face
  {
  public:
    Vertex* vertices[3];
  };

  class Mesh : public Resource
  {
  public:
    Mesh();
    Mesh(String file);
    virtual ~Mesh();

    virtual void Init(bool flushClientSideArray = true) override;
    virtual void UnInit() override;
    virtual void Load() override;
    virtual void Save(bool onlyIfDirty) override;
    virtual int GetVertexSize() const;
    virtual bool IsSkinned() const;
    void CalculateAABoundingBox();
    void GetAllMeshes(MeshRawPtrArray& meshes);
    void GetAllMeshes(MeshRawCPtrArray& meshes) const;
    void ConstructFaces();
    void ApplyTransform(const Mat4& transform);

    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  protected:
    virtual void InitVertices(bool flush);
    virtual void InitIndices(bool flush);
    void UpdateAABB(const Vec3& v);

  protected:
    virtual void CopyTo(Resource* other) override;

  public:
    VertexArray m_clientSideVertices;
    std::vector<uint> m_clientSideIndices;
    GLuint m_vboVertexId = 0;
    GLuint m_vboIndexId = 0;
    GLuint m_vaoId = 0;
    uint m_vertexCount = 0;
    uint m_indexCount = 0;
    MaterialPtr m_material;
    MeshPtrArray m_subMeshes;
    BoundingBox m_aabb;
    FaceArray m_faces;

  private:
    MeshRawPtrArray m_allMeshes;
  };

  class SkinVertex : public Vertex
  {
  public:
    UVec4 bones;
    Vec4 weights;
  };

  class SkinMesh : public Mesh
  {
  public:
    SkinMesh();
    SkinMesh(String file);
    ~SkinMesh();

    virtual void Init(bool flushClientSideArray = true) override;
    virtual void UnInit() override;
    virtual void Load() override;

    virtual int GetVertexSize() const override;
    virtual bool IsSkinned() const override;

  protected:
    virtual void InitVertices(bool flush) override;

  public:
    std::vector<SkinVertex> m_clientSideVertices;
    class Skeleton* m_skeleton;
  };

  class MeshManager : public ResourceManager
  {
  public:
    MeshManager();
    virtual ~MeshManager();
  };

}
