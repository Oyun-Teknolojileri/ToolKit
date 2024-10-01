/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

/*
 * References:
 * https://box2d.org/files/ErinCatto_DynamicBVH_Full.pdf
 * Implementation is based on (MIT Licensed):
 * https://github.com/Sopiro/Muli/blob/master/include/muli/aabb_tree.h
 */

#pragma once

#include "GeometryTypes.h"

namespace ToolKit
{

  typedef int AABBNodeProxy;
  typedef std::unordered_set<AABBNodeProxy> AABBNodeProxySet;
  typedef std::vector<AABBNodeProxy> NodeProxyArray;

  class TK_API AABBTree
  {
   public:
    static constexpr inline int32 nullNode = -1;

    struct AABBNode
    {
      bool IsLeaf() const { return child1 == nullNode; }

      BoundingBox aabb;
      EntityWeakPtr entity;

      AABBNodeProxy parent;
      AABBNodeProxy child1;
      AABBNodeProxy child2;
      AABBNodeProxy next;
      bool moved;

      AABBNodeProxySet leafs;
    };

    typedef std::vector<AABBNode> AABBNodeArray;

   public:
    AABBTree();
    ~AABBTree();

    AABBTree(const AABBTree&)            = delete;
    AABBTree& operator=(const AABBTree&) = delete;

    void Reset();
    AABBNodeProxy CreateNode(EntityWeakPtr entity, const BoundingBox& aabb);
    /** Updates the tree via reinserting the provided node. */
    void UpdateNode(AABBNodeProxy node);
    void RemoveNode(AABBNodeProxy node);
    void Traverse(std::function<void(const AABBNode*)> callback) const;

    /** Creates an optimum aabb tree in bottom up fashion but its very slow to use even at scene loading.  */
    void Rebuild();

    /** Return debug boxes for each node in the tree. */
    void GetDebugBoundingBoxes(EntityPtrArray& boundingBoxes) const;

    /** Returns the bounding box that covers all entities. */
    const BoundingBox& GetRootBoundingBox() const;

    /** Template for volume queries. VolumeTypes: {Frustum, BoundingBox} */
    template <typename VolumeType>
    EntityRawPtrArray VolumeQuery(const VolumeType& vol) const;

    /**
     * Test ray against the tree and returns the nearest entity that hits the ray and the hit distance t.
     * If the deep parameter passed as true, it checks mesh level intersection.
     */
    EntityPtr RayQuery(const Ray& ray, bool deep, float* t = nullptr) const;

   private:
    AABBNodeProxy AllocateNode();
    void FreeNode(AABBNodeProxy node);
    AABBNodeProxy InsertLeaf(AABBNodeProxy leaf);
    void RemoveLeaf(AABBNodeProxy leaf);
    void Rotate(AABBNodeProxy node);

    void VolumeQuery(EntityRawPtrArray& result,
                     std::atomic_int& threadCount,
                     AABBNodeProxy root,
                     std::function<enum class IntersectResult(AABBNodeProxy)> queryFn) const;

   private:
    AABBNodeProxy m_root;
    AABBNodeProxy m_freeList;

    AABBNodeArray m_nodes;
    int m_nodeCapacity;
    int m_nodeCount;

    /** Threshold node count to do threaded traverse for volume queries. */
    const int m_threadTreshold;

    /** Internally used to get max available thread count. */
    mutable int m_maxThreadCount;
  };

} // namespace ToolKit
