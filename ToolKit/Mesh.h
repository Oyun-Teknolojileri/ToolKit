#pragma once

#include "MathUtil.h"
#include "RenderState.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "Types.h"

#include <memory>
#include <vector>

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

  class TK_API Mesh : public Resource
  {
   public:
    TKResourceType(Mesh)

    Mesh();
    explicit Mesh(const String& file);
    virtual ~Mesh();

    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;
    void Load() override;
    void Save(bool onlyIfDirty) override;
    virtual int GetVertexSize() const;
    virtual bool IsSkinned() const;
    void CalculateAABB();
    void GetAllMeshes(MeshRawPtrArray& meshes);
    void GetAllMeshes(MeshRawCPtrArray& meshes) const;
    void ConstructFaces();
    void ApplyTransform(const Mat4& transform);

    void SetMaterial(MaterialPtr material);

    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    /**
     * Traverse all submeshes recursively.
     * @param callback is the function to call on each mesh.
     * @param mesh is this or submesh. Pass null to start iteration from this
     * mesh.
     */
    void TraverseAllMesh(std::function<void(Mesh*)> callback,
                         Mesh* mesh = nullptr);

    /**
     * Const traverse all submeshes recursively. Refer TraverseAllMesh for
     * details.
     */
    void TraverseAllMesh(std::function<void(const Mesh*)> callback,
                         const Mesh* mesh = nullptr) const;

   protected:
    virtual void InitVertices(bool flush);
    virtual void InitIndices(bool flush);
    void CopyTo(Resource* other) override;

   public:
    VertexArray m_clientSideVertices;
    UIntArray m_clientSideIndices;
    uint m_vboVertexId = 0;
    uint m_vboIndexId  = 0;
    uint m_vaoId       = 0;
    uint m_vertexCount = 0;
    uint m_indexCount  = 0;
    MaterialPtr m_material;
    MeshPtrArray m_subMeshes;
    BoundingBox m_aabb;
    FaceArray m_faces;
    VertexLayout m_vertexLayout;

   private:
    MeshRawPtrArray m_allMeshes;
  };

  class SkinVertex : public Vertex
  {
   public:
    Vec4 bones;
    Vec4 weights;
  };

  class TK_API SkinMesh : public Mesh
  {
   public:
    TKResourceType(SkinMesh)

    SkinMesh();
    explicit SkinMesh(const String& file);
    ~SkinMesh();

    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;
    void Load() override;

    int GetVertexSize() const override;
    bool IsSkinned() const override;

    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    // Because AABB is all dependent on active animation, just return AABB
    // (doesn't change m_aabb)
    BoundingBox CalculateAABB(const Skeleton* skel,
                              const DynamicBoneMap* boneMap);

   protected:
    void InitVertices(bool flush) override;
    void CopyTo(Resource* other) override;

   public:
    std::vector<SkinVertex> m_clientSideVertices;
    SkeletonPtr m_skeleton;
  };

  class TK_API MeshManager : public ResourceManager
  {
   public:
    MeshManager();
    virtual ~MeshManager();
    bool CanStore(ResourceType t) override;
    ResourcePtr CreateLocal(ResourceType type) override;
    String GetDefaultResource(ResourceType type) override;
  };

  void SetVertexLayout(VertexLayout layout);
} // namespace ToolKit
