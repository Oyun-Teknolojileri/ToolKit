/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "EngineSettings.h"
#include "GeometryTypes.h"
#include "MathUtil.h"
#include "Scene.h"
#include "Threads.h"

namespace ToolKit
{

  // BVHNode
  //////////////////////////////////////////

  class TK_API BVHNode
  {
   public:
    BVHNode();
    ~BVHNode();

    int depth           = 0;

    /** Parent bvh node. */
    BVHNodePtr m_parent = nullptr;
    /** Left bvh node. */
    BVHNodePtr m_left   = nullptr;
    /** Right bvh node. */
    BVHNodePtr m_right  = nullptr;
    /** Boundary of the node. */
    BoundingBox m_aabb;
    /** All entities inside this node. */
    EntityPtrArray m_entites;
    /** All lights intersecting with this node. */
    LightPtrArray m_lights;
    /** Internal variable. True if this node is going to be deleted by conjunction. */
    bool m_markedForDelete              = false;
    /** Holds the last query results. Preserve its state until the next query. */
    IntersectResult m_frustumTestResult = IntersectResult::Outside;

    /** Evaluates last frustum query results and returns true if IntersectionResult is Inside. */
    bool IsInsideFrustum() const { return m_frustumTestResult == IntersectResult::Inside; }

    /** Evaluates last frustum query results and returns true if IntersectionResult is Inside. */
    bool IsIntersectFrustum() const { return m_frustumTestResult == IntersectResult::Intersect; }

    /** Evaluates last frustum query results and returns true if IntersectionResult is Outside. */
    bool IsOutsideFrustum() const { return m_frustumTestResult == IntersectResult::Outside; }

    /** Checks if the node is a leaf node. It is guaranteed that if left node is there, there will be right node too. */
    bool IsLeaf() const { return m_left == nullptr; }

    /** Checks wheter the node is root or not. Its considered a root node if it has no parent. */
    bool IsRoot() const { return m_parent == nullptr; }
  };

  // BVHTree
  //////////////////////////////////////////

  class TK_API BVHTree
  {
   public:
    BVHTree(BVHPtr owner);
    ~BVHTree();

    // Return true if rebuilding the bvh necessary
    bool Add(EntityPtr entity);
    void Remove(EntityPtr entity);
    void Clean();
    void UpdateLeaf(BVHNodePtr node, bool removedFromThisNode);

   private:
    BVHTree() = delete;

    void ReAssignLightsFromParent(BVHNodePtr node);

   public:
    BVHNodePtr m_root = nullptr;
    std::vector<BVHNodePtr> m_nodesToDelete;

    // Utility queue that used to iterate nodes
    std::deque<BVHNodePtr> m_nextNodes;

    int m_maxEntityCountPerBVHNode = 10;
    float m_minBBSize              = 0.0f;
    int m_maxDepth                 = 100;

   private:
    BVHPtr m_bvh = nullptr;
  };

  // BVH
  //////////////////////////////////////////

  class TK_API BVH : public Object
  {
    friend BVHTree;

   public:
    BVH();
    virtual ~BVH();
    virtual void NativeConstruct(ScenePtr scene);

    // IMPORTANT: This should be called before building the BVH (or call ReBuild after this function) otherwise
    // inconsistencies will occur in BVH nodes.
    void SetParameters(const EngineSettings::GraphicSettings& settings);

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
                    bool pickPartiallyInside);

    /**
     * Test bvh with a frustum and assign test results to BVHNodes.
     * Results are valid as long as another test or call to any query functions has not been done. In short, results
     * should be used after query performed.
     */
    void FrustumTest(const Frustum& frustum, EntityRawPtrArray& entities);

    // Debug functions.
    void GetDebugBVHBoxes(EntityPtrArray& boxes); //!< Creates a debug box for each leaf node in the bvh.

    /**
     * A quality measurement function for bvh settings. assignmentPerNtt must be close to 1. Higher ratio means the same
     * ntt is being assigned to multiple bvh leafs.
     */
    void DistributionQuality(int& totalNtties, int& assignedNtties, float& assignmentPerNtt);

    const BoundingBox& GetBVHBoundary();

   public:
    BVHTreePtr m_bvhTree = nullptr;

   private:
    ScenePtr m_scene = nullptr;

    AtomicLock m_addLock;
    EntityPtrArray m_entitiesToAdd;

    AtomicLock m_removeLock;
    EntityPtrArray m_entitiesToRemove;

    AtomicLock m_updateLock;
    EntityPtrArray m_entitiesToUpdate;
  };

} // namespace ToolKit
