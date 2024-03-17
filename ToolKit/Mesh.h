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

  /**
   * @class Vertex
   * @brief Represents a single vertex in a mesh.
   *
   * This class encapsulates the properties of a vertex including its position,
   * normal vector, texture coordinates, and binormal vector.
   */
  class Vertex
  {
   public:
    Vec3 pos;  //!< Position of the vertex in 3D space.
    Vec3 norm; //!< Normal vector of the vertex.
    Vec2 tex;  //!< Texture coordinates of the vertex.
    Vec3 btan; //!< Binormal (bitangent) vector of the vertex.
  };

  /**
   * @class Face
   * @brief Represents a single face in a mesh.
   *
   * This class holds an array of pointers to vertices that make up a face.
   * Typically, a face is a triangle consisting of three vertices.
   */
  class Face
  {
   public:
    Vertex* vertices[3]; //!< Pointers to the vertices that form the face.
  };

  /**
   * @class Mesh
   * @brief Represents a 3D mesh.
   *
   * Inherits from Resource and provides functionality for managing and manipulating
   * mesh data including vertices, faces, and materials.
   */
  class TK_API Mesh : public Resource
  {
   public:
    TKDeclareClass(Mesh, Resource);

    /**
     * @brief Default constructor for Mesh.
     *
     * Initializes a new instance of the Mesh class with default parameters.
     */
    Mesh();

    /**
     * @brief Parameterized constructor for Mesh.
     *
     * Initializes a new instance of the Mesh class, sets the resource file to be used during load.
     * @param file The path to the file containing the mesh data.
     */
    explicit Mesh(const String& file);

    /**
     * @brief Virtual destructor for Mesh.
     *
     * Cleans up any resources used by the Mesh instance before destruction.
     */
    virtual ~Mesh();

    /**
     * @brief Initializes the mesh, optionally flushing the client-side array.
     *
     * This function prepares the mesh for use, setting up any necessary data structures or state.
     * @param flushClientSideArray If true, the client-side array is flushed during initialization.
     */
    void Init(bool flushClientSideArray = false) override;

    /**
     * @brief Uninitializes the mesh, releasing any allocated resources.
     *
     * This function is called when the mesh is no longer needed, and it cleans up any resources that were allocated.
     */
    void UnInit() override;

    /**
     * @brief Loads the mesh data.
     *
     * This function is responsible for loading mesh data from an external source or file.
     */
    void Load() override;

    /**
     * @brief Saves the mesh data, with an option to save only if modifications have been made.
     *
     * This function saves the current state of the mesh to an external source or file.
     * @param onlyIfDirty If true, the mesh is saved only if changes have been detected.
     */
    void Save(bool onlyIfDirty) override;

    /**
     * @brief Retrieves the size of a single vertex.
     *
     * This function calculates and returns the size, in bytes, of a single vertex in the mesh.
     * @return The size of a single vertex in bytes.
     */
    virtual int GetVertexSize() const;

    /**
     * @brief Retrieves the total number of vertices in the mesh.
     *
     * This function counts and returns the total number of vertices across all submeshes.
     * @return The total number of vertices as an unsigned integer.
     */
    virtual uint GetVertexCount() const;

    /**
     * @brief Determines if the mesh is skinned.
     *
     * This function checks if the mesh contains skinning information for animations.
     * @return True if the mesh is skinned, false otherwise.
     */
    virtual bool IsSkinned() const;

    /**
     * @brief Calculates and updates the bounding box for all meshes and submeshes.
     *
     * After calling this function, m_boundingBox will contain the updated bounds
     * based on the vertices in the client-side array.
     */
    void CalculateAABB();

    /**
     * @brief Accumulate all meshes and sub meshes recursively and returns the flattened array.
     * If Mesh is modified consider sending updateCache, which will re accumulate the mesh array before returning it.
     * @param meshes Reference to an array of MeshRawPtr to store the accumulated meshes.
     * @param updateCache If true, the mesh array will be re-accumulated before returning.
     */
    void GetAllMeshes(MeshRawPtrArray& meshes, bool updateCache = false) const;

    /**
     * @brief Accumulate all submeshes of the current mesh recursively.
     * This function traverses through all the sub-meshes of the current mesh and accumulates them into the provided
     * array.
     * @param meshes Reference to an array of MeshPtr to store the accumulated sub-meshes.
     */
    void GetAllSubMeshes(MeshPtrArray& meshes) const;

    /**
     * @brief Retrieve all meshes.
     *
     * Provides access to the array of all meshes and submeshes that have been accumulated.
     * This is a read-only accessor that returns a constant reference to the internal mesh array.
     *
     * @return A constant reference to the MeshRawPtrArray containing all meshes.
     */
    const MeshRawPtrArray& GetAllMeshes() const;

    /**
     * @brief Get the total number of meshes and submeshes.
     *
     * This function counts all the meshes and submeshes contained within the class instance.
     *
     * @return The count of meshes and submeshes as an integer.
     */
    int GetMeshCount() const;

    /**
     * @brief Calculate the total number of vertices across all meshes.
     *
     * This virtual function computes the sum of vertices from all meshes and submeshes.
     *
     * @return The total vertex count as an unsigned integer.
     */
    virtual uint TotalVertexCount() const;

    /**
     * @brief Construct face geometry from indices and vertex data.
     *
     * This function processes index and vertex data to form the faces of the mesh,
     * updating the internal face structure.
     */
    void ConstructFaces();

    /**
     * @brief Apply a transformation matrix to mesh vertices.
     *
     * This function multiplies the vertices of the mesh by the provided transformation matrix,
     * altering their position, rotation, and scale according to the matrix.
     *
     * @param transform The transformation matrix to apply to the mesh vertices.
     */
    void ApplyTransform(const Mat4& transform);

    /**
     * @brief Set the initial material for this mesh.
     *
     * Assigns a material to the mesh. If a Material component is present, it will take precedence
     * over the material set by this function.
     *
     * @param material A pointer to the Material to assign to this mesh.
     */
    void SetMaterial(MaterialPtr material);

   protected:
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

    /**
     * @brief Initializes the vertex data.
     * @param flush If true, existing client-side vertex data is flushed.
     */
    virtual void InitVertices(bool flush);

    /**
     * @brief Initializes the index data.
     * @param flush If true, existing client-side index data is flushed.
     */
    virtual void InitIndices(bool flush);

    /**
     * @brief Copies this mesh's data to another mesh resource.
     * @param other Pointer to the Resource to copy data to.
     */
    void CopyTo(Resource* other) override;

   public:
    VertexArray m_clientSideVertices; //!< Array of vertices stored on the client side.
    UIntArray m_clientSideIndices;    //!< Array of indices stored on the client side.
    uint m_vboVertexId = 0;           //!< ID of the vertex buffer object.
    uint m_vboIndexId  = 0;           //!< ID of the index buffer object.
    uint m_vaoId       = 0;           //!< ID of the vertex array object.
    uint m_vertexCount = 0;           //!< Count of vertices.
    uint m_indexCount  = 0;           //!< Count of indices.
    MaterialPtr m_material;           //!< Pointer to the material used by the mesh.
    MeshPtrArray m_subMeshes;         //!< Array of pointers to submeshes.
    BoundingBox m_boundingBox;        //!< Bounding box of the mesh.
    FaceArray m_faces;                //!< Array of faces that make up the mesh.
    VertexLayout m_vertexLayout;      //!< Layout of the vertices.

   protected:
    mutable MeshRawPtrArray m_allMeshes; //!< Cached array of all meshes including submeshes.
  };

  /**
   * @class SkinVertex
   * @brief Represents a skinned vertex that includes bone and weight information for skeletal animation.
   *
   * This class extends the Vertex class by adding skinning information, which consists of bone indices and weights.
   */
  class SkinVertex : public Vertex
  {
   public:
    Vec4 bones;   //!< Indices of the bones that affect this vertex.
    Vec4 weights; //!< Weights corresponding to the influence of each bone on this vertex.
  };

  /**
   * @class SkinMesh
   * @brief Represents a 3D skinned mesh that is capable of skeletal animation.
   *
   * Inherits from Mesh and adds functionality for managing skinning information related to vertices and bones.
   */
  class TK_API SkinMesh : public Mesh
  {
   public:
    TKDeclareClass(SkinMesh, Mesh);

    /**
     * @brief Default constructor for SkinMesh.
     *
     * Initializes a new instance of the SkinMesh class with default parameters.
     */
    SkinMesh();

    /**
     * @brief Parameterized constructor for SkinMesh.
     *
     * Initializes a new instance of the SkinMesh class, sets the resource file to be used during load.
     * @param file The path to the file containing the mesh data.
     */
    explicit SkinMesh(const String& file);

    /**
     * @brief Virtual destructor for SkinMesh.
     *
     * Cleans up any resources used by the SkinMesh instance before destruction.
     */
    virtual ~SkinMesh();

    /**
     * @brief Initializes the skin mesh, optionally flushing the client-side array.
     *
     * This function prepares the skin mesh for use, setting up any necessary data structures or state.
     * @param flushClientSideArray If true, the client-side array is flushed during initialization.
     */
    void Init(bool flushClientSideArray = false) override;

    /**
     * @brief Uninitializes the skin mesh, releasing any allocated resources.
     *
     * This function is called when the skin mesh is no longer needed, and it cleans up any resources that were
     * allocated.
     */
    void UnInit() override;

    /**
     * @brief Loads the skin mesh data.
     *
     * This function is responsible for loading skin mesh data from an external source or file.
     */
    void Load() override;

    /**
     * @brief Retrieves the total number of vertices in the skin mesh.
     *
     * This function counts and returns the total number of vertices across all submeshes, including skinning
     * information.
     * @return The total number of vertices as an unsigned integer.
     */
    virtual uint TotalVertexCount() const override;

    /**
     * @brief Retrieves the number of vertices in the skin mesh.
     *
     * This function returns the number of vertices in the skin mesh, excluding any submeshes.
     * @return The number of vertices as an unsigned integer.
     */
    uint GetVertexCount() const override;

    /**
     * @brief Retrieves the size of a single skinned vertex.
     *
     * This function calculates and returns the size, in bytes, of a single skinned vertex in the mesh.
     * @return The size of a single skinned vertex in bytes.
     */
    int GetVertexSize() const override;

    /**
     * @brief Determines if the mesh is a skin mesh.
     *
     * This function checks if the mesh contains skinning information for skeletal animations.
     * @return True if the mesh is a skin mesh, false otherwise.
     */
    bool IsSkinned() const override;

    /**
     * @brief Calculates the axis-aligned bounding box (AABB) based on the current pose of the skeleton.
     *
     * This function computes the AABB for the mesh as influenced by the given skeleton and bone map.
     * It does not alter the mesh's original bounding box (m_boundingBox).
     * @param skel Pointer to the Skeleton influencing the mesh.
     * @param boneMap Pointer to the DynamicBoneMap containing bone mapping information.
     * @return The calculated AABB for the current pose.
     */
    BoundingBox CalculateAABB(const Skeleton* skel, DynamicBoneMapPtr boneMap);

   protected:
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

    /**
     * @brief Initializes the vertex data of the mesh.
     *
     * This function sets up the vertex data for the mesh, with an option to flush existing data.
     * @param flush If true, existing vertex data is cleared before initialization.
     */
    void InitVertices(bool flush) override;

    /**
     * @brief Copies the mesh data to another resource.
     *
     * This function duplicates the mesh's data into another mesh resource.
     * @param other The resource to which the mesh data will be copied.
     */
    void CopyTo(Resource* other) override;

   public:
    std::vector<SkinVertex> m_clientSideVertices; //!< Array of vertices with skinning information.
    SkeletonPtr m_skeleton;                       //!< Pointer to the skeleton associated with this mesh.
    bool m_bindPoseAABBCalculated = false;        //!< Flag indicating if the bind pose AABB has been calculated.
    BoundingBox m_bindPoseAABB;                   //!< The AABB of the mesh in its bind pose.
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
