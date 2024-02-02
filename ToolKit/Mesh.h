/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "RenderState.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "Types.h"

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
    TKDeclareClass(Mesh, Resource);

    Mesh();
    explicit Mesh(const String& file);
    virtual ~Mesh();

    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;
    void Load() override;
    void Save(bool onlyIfDirty) override;
    virtual int GetVertexSize() const;
    virtual uint GetVertexCount() const;
    virtual bool IsSkinned() const;

    /**
     * Calculates a bounding box for vertices in client side array for all meshes and submeshes. m_aabb becomes
     * valid after call to this.
     */
    void CalculateAABB();

    /**
     * Accumulate all meshes and sub meshes recursively and returns the flattened array.
     * If class is modified consider sending updateCache, which will re accumulate the mesh array before sending it.
     */
    void GetAllMeshes(MeshRawPtrArray& meshes, bool updateCache = false) const;

    const MeshRawPtrArray& GetAllMeshes() const { return m_allMeshes; }

    /**
     * Return the number of meshes and sub meshes in this class.
     */
    int GetMeshCount() const;

    virtual uint TotalVertexCount() const;

    /**
     * Construct faces from index and vertex data.
     */
    void ConstructFaces();

    /**
     * Applies the given transform to the vertices at the client side array.
     */
    void ApplyTransform(const Mat4& transform);

    /**
     * Sets the material assigned to this mesh initially. Material component overrides this if present.
     */
    void SetMaterial(MaterialPtr material);

   protected:
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

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

   protected:
    mutable MeshRawPtrArray m_allMeshes;
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
    TKDeclareClass(SkinMesh, Mesh);

    SkinMesh();
    explicit SkinMesh(const String& file);
    virtual ~SkinMesh();

    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;
    void Load() override;

    virtual uint TotalVertexCount() const override;
    uint GetVertexCount() const override;
    int GetVertexSize() const override;
    bool IsSkinned() const override;

    // Because AABB is all dependent on active animation, just return AABB
    // (doesn't change m_aabb)
    BoundingBox CalculateAABB(const Skeleton* skel, DynamicBoneMapPtr boneMap);

   protected:
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

    void InitVertices(bool flush) override;
    void CopyTo(Resource* other) override;

   public:
    std::vector<SkinVertex> m_clientSideVertices;
    SkeletonPtr m_skeleton;
    bool m_bindPoseAABBCalculated = false;
    BoundingBox m_bindPoseAABB;
  };

  class TK_API MeshManager : public ResourceManager
  {
   public:
    MeshManager();
    virtual ~MeshManager();
    bool CanStore(ClassMeta* Class) override;
    String GetDefaultResource(ClassMeta* Class) override;
  };

} // namespace ToolKit
