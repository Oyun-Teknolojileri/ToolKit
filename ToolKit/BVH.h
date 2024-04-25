/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "GeometryTypes.h"
#include "MathUtil.h"
#include "Scene.h"
#include "Threads.h"

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
    LightPtrArray m_lights;

    bool m_waitingForDeletion = false; //<! Internal variable. True if this node is going to be deleted by conjunction.
    IntersectResult m_frustumTestResult = IntersectResult::Outside;

    /**
     * It is guaranteed that if left node is there, there will be right node too
     */
    inline bool Leaf() const { return m_left == nullptr; }

    bool m_insideFrustum = false;
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
    void UpdateLeaf(BVHNode* node, bool removedFromThisNode);

   private:
    BVHTree() = delete;

    void ReAssignLightsFromParent(BVHNode* node);

   public:
    BVHNode* m_root = nullptr;
    std::vector<BVHNode*> m_nodesToDelete;

    // Utility queue that used to iterate nodes
    std::deque<BVHNode*> m_nextNodes;

    int m_maxEntityCountPerBVHNode = 10;
    float m_minBBSize              = 0.0f;
    int m_maxDepth                 = 100;

   private:
    class BVH* m_bvh = nullptr;
  };

  class TK_API BVH
  {
    friend BVHTree;

   public:
    BVH(Scene* scene);
    ~BVH();

    // IMPORTANT: This should be called before building the BVH (or call ReBuild after this function) otherwise
    // inconsistencies will occur in BVH nodes.
    void SetParameters(const EngineSettings::PostProcessingSettings& settings);

    void ReBuild();
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

    BoundingBox GetFrustumBoundary(const Frustum& frustum) const;

    /**
     * Test bvh with a frustum and assign test results to BVHNodes.
     * Results are valid as long as another test or call to any query functions has not been done. In short, results
     * should be used after query performed.
     */
    void FrustumTest(const Frustum& frustum, EntityRawPtrArray& entities);

    // Debug functions.
    void GetDebugBVHBoxes(EntityPtrArray& boxes); //!< Creates a debug box for each leaf node in the bvh.
    void SanityCheck();                           //!< // Checks if any entity holds a node that is not leaf.

    /**
     * A quality measurement function for bvh settings. assignmentPerNtt must be close to 1. Higher ratio means the same
     * ntt is being assigned to multiple bvh leafs.
     */
    void DistributionQuality(int& totalNtties, int& assignedNtties, float& assignmentPerNtt);

    const BoundingBox& GetBVHBoundary();

   public:
    BVHTree* m_bvhTree = nullptr;

   private:
    BVH()          = delete;
    Scene* m_scene = nullptr;

    AtomicLock m_addLock;
    EntityPtrArray m_entitiesToAdd;

    AtomicLock m_removeLock;
    EntityPtrArray m_entitiesToRemove;

    AtomicLock m_updateLock;
    EntityPtrArray m_entitiesToUpdate;
  };

} // namespace ToolKit
