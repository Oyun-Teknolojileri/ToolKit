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

#include "MathUtil.h"
#include "Types.h"

namespace ToolKit
{

  typedef int32 NodeProxy;

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
    };

    typedef std::vector<AABBTree::Node> NodeArray;

   public:
    AABBTree();
    ~AABBTree();

    AABBTree(const AABBTree&)            = delete;
    AABBTree& operator=(const AABBTree&) = delete;

    void Reset();
    NodeProxy CreateNode(EntityWeakPtr entity, const BoundingBox& aabb);
    void UpdateNode(NodeProxy node); /** Updates the tree via reinserting the provided node. */
    void RemoveNode(NodeProxy node);
    void Rebuild();
    void GetDebugBoundingBoxes(EntityPtrArray& boundingBoxes);

   private:
    NodeProxy AllocateNode();
    void FreeNode(NodeProxy node);
    NodeProxy InsertLeaf(NodeProxy leaf);
    void RemoveLeaf(NodeProxy leaf);
    void Rotate(NodeProxy node);

   private:
    NodeProxy root;

    NodeArray nodes;
    int32 nodeCapacity;
    int32 nodeCount;

    NodeProxy freeList;
  };

} // namespace ToolKit
