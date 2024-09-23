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

  typedef int32 NodeProxy;
  typedef std::unordered_set<NodeProxy> NodeProxySet;

  const Vec3 aabb_margin          = Vec3(0.03f);
  constexpr float aabb_multiplier = 3.0f;

  class TK_API AABBTree
  {
   public:
    static constexpr inline int32 nullNode = -1;

    struct Node
    {
      bool IsLeaf() const { return child1 == nullNode; }

      BoundingBox aabb;
      EntityWeakPtr entity;

      NodeProxy parent;
      NodeProxy child1;
      NodeProxy child2;
      NodeProxy next;
      bool moved;

      NodeProxySet leafs;
    };

    typedef std::vector<AABBTree::Node> NodeArray;

   public:
    AABBTree();
    ~AABBTree();

    AABBTree(const AABBTree&)            = delete;
    AABBTree& operator=(const AABBTree&) = delete;

    void Reset();
    NodeProxy CreateNode(EntityWeakPtr entity, const BoundingBox& aabb);
    /** Updates the tree via reinserting the provided node. */
    void UpdateNode(NodeProxy node);
    void RemoveNode(NodeProxy node);
    void Traverse(std::function<void(const Node*)> callback) const;
    /** Creates an optimum aabb tree in bottom up fashion but its very slow to use even at scene loading.  */
    void Rebuild();
    
    /** Return debug boxes for each node in the tree. */
    void GetDebugBoundingBoxes(EntityPtrArray& boundingBoxes) const;

    /** Prints each node and the leaf entities under the node to the console. */
    void PrintTree();

    const BoundingBox& GetRootBoundingBox() const;

    /** Checks frustum against the tree and returns the entities intersecting with the frustum. */
    EntityRawPtrArray FrustumQuery(const Frustum& frustum) const;

    /**
     * Checks entities inside the aabb tree and return the nearest one that hits the ray and the hit distance t.
     * If the deep parameter passed as true, it checks mesh level intersection.
     */
    EntityPtr RayQuery(const Ray& ray, bool deep, float* t = nullptr) const;

   private:
    NodeProxy AllocateNode();
    void FreeNode(NodeProxy node);
    NodeProxy InsertLeaf(NodeProxy leaf);
    void RemoveLeaf(NodeProxy leaf);
    void Rotate(NodeProxy node);

    void TraverseParallel(const Frustum& frustum, EntityRawPtrArray& unculled, NodeProxy root, std::atomic<int>& thredCount) const;

   private:
    NodeProxy root;

    NodeArray nodes;
    int32 nodeCapacity;
    int32 nodeCount;

    NodeProxy freeList;
  };

} // namespace ToolKit
