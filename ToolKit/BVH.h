#pragma once

#include "GeometryTypes.h"
#include "Scene.h"

#include <deque>

namespace ToolKit
{

  class TK_API BVHNode
  {
   public:
    BVHNode();
    ~BVHNode();

    int depth         = 0;

    BVHNode* m_parent = nullptr;
    BVHNode* m_left   = nullptr;
    BVHNode* m_right  = nullptr;

    BoundingBox m_aabb;

    EntityPtrArray m_entites;
    EntityPtrArray m_lights;

    bool m_waitingForDeletion = false; //<! Internal variable. True if this node is going to be deleted by conjunction.
    bool m_insideFrustum      = false;

    /**
     * It is guaranteed that if left node is there, there will be right node too
     */
    inline bool Leaf() const { return m_left == nullptr; }
  };

  class TK_API BVHTree
  {
   public:
    BVHTree(class BVH* owner);
    ~BVHTree();

    // Return true if rebuilding the bvh necessary
    bool Add(EntityPtr& entity);
    void Remove(EntityPtr& entity);
    void Clean();

   private:
    BVHTree() = delete;

    void UpdateLeaf(BVHNode* node);

   public:
    BVHNode* m_root = nullptr;
    std::vector<BVHNode*> m_nodesToDelete;

    // Utility queue that used to iterate nodes
    std::deque<BVHNode*> m_nextNodes;

    int m_maxEntityCountPerBVHNode = 10;
    float m_minBBSize              = 2.0f;

   private:
    class BVH* m_bvh = nullptr;
  };

  class TK_API BVH
  {
   public:
    BVH(Scene* scene);
    ~BVH();

    // IMPORTANT: This should be called before building the BVH (or call ReBuild after this function) otherwise
    // inconsistencies will occur in BVH nodes.
    void SetParameters(const EngineSettings::PostProcessingSettings& settings);

    bool ReBuild();
    void Clean();

    void AddEntity(const EntityPtr& entity);
    void RemoveEntity(const EntityPtr& entity);
    void UpdateEntity(const EntityPtr& entity);
    void Update();

    // Query functions
    void PickObject(const Ray& ray, Scene::PickData& pickData, const IDArray& ignoreList, float& closestDistance);
    void PickObject(const Frustum& frustum,
                    Scene::PickDataArray& pickedObjects,
                    const IDArray& ignoreList,
                    const EntityPtrArray& extraList,
                    bool pickPartiallyInside);

    // Debug functions
    void GetDebugBVHBoxes(EntityPtrArray& boxes);
    void SanityCheck();

   private:
    void UpdateBoundary(); // Update the bounding box via traversing each leaf.

   public:
    BVHTree* m_bvhTree = nullptr;
    BoundingBox m_boundingBox; // Boundingbox that covers bvh nodes.

   private:
    BVH()          = delete;
    Scene* m_scene = nullptr;
    EntityPtrArray m_entitiesToAdd;
    EntityPtrArray m_entitiesToRemove;
    EntityPtrArray m_entitiesToUpdate;
  };

} // namespace ToolKit
