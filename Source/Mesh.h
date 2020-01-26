#pragma once

#include "Types.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "GL/glew.h"
#include <memory>

namespace ToolKit
{

  class Material;

  class Vertex
  {
  public:
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 tex;
    glm::vec3 btan;
  };

  class Mesh : public Resource
  {
  public:
    Mesh();
    Mesh(std::string file);
    virtual ~Mesh();

    virtual void Init(bool flushClientSideArray = true) override;
		virtual void UnInit() override;
    virtual void Load() override;
    virtual int GetVertexSize();
    virtual bool IsSkinned();

  protected:
    virtual void InitVertices(bool flush);
    virtual void InitIndices(bool flush);

  public:
    std::vector<Vertex> m_clientSideVertices;
    std::vector<uint> m_clientSideIndices;
    GLuint m_vboVertexId = 0;
    GLuint m_vboIndexId = 0;
    uint m_vertexCount = 0;
    uint m_indexCount = 0;
    std::shared_ptr<Material> m_material;
    std::vector<Mesh*> m_subMeshes;
  };

  class MeshManager : public ResourceManager<Mesh>
  {
  };

  class SkinVertex : public Vertex
  {
  public:
    glm::uvec4 bones;
    glm::vec4 weights;
  };

  class Skeleton;

  class SkinMesh : public Mesh
  {
  public:
    SkinMesh();
    SkinMesh(std::string file);
    ~SkinMesh();

    virtual void Init(bool flushClientSideArray = true) override;
		virtual void UnInit() override;
    virtual void Load() override;

    int GetVertexSize();
    bool IsSkinned();

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
