#pragma once

#include "Types.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "MathUtil.h"
#include "GL/glew.h"
#include <memory>

namespace ToolKit
{

  class Material;

  class Vertex
  {
  public:
    Vec3 pos;
    Vec3 norm;
    Vec2 tex;
    Vec3 btan;
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
    virtual Mesh* GetCopy() override;
    virtual int GetVertexSize() const;
    virtual bool IsSkinned() const;
		void CalculateAABoundingBox();
		void GetAllMeshes(MeshRawPtrArray& meshes);
    void Scale(const Vec3& scale);

  protected:
    virtual void InitVertices(bool flush);
    virtual void InitIndices(bool flush);
		void UpdateAABB(const Vec3& v);

  public:
    VertexArray m_clientSideVertices;
    std::vector<uint> m_clientSideIndices;
    GLuint m_vboVertexId = 0;
    GLuint m_vboIndexId = 0;
    uint m_vertexCount = 0;
    uint m_indexCount = 0;
    MaterialPtr m_material;
    MeshPtrArray m_subMeshes;
		BoundingBox m_aabb;

  private:
    MeshRawPtrArray m_allMeshes;
  };

  class MeshManager : public ResourceManager<Mesh>
  {
  };

  class SkinVertex : public Vertex
  {
  public:
    glm::uvec4 bones;
    Vec4 weights;
  };

  class Skeleton;

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
    virtual void InitVertices(bool flush);

  public:
    std::vector<SkinVertex> m_clientSideVertices;
    Skeleton* m_skeleton;
  };

  class SkinMeshManager : public ResourceManager<SkinMesh>
  {
  };

}
