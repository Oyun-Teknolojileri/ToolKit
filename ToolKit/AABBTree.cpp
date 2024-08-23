/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "AABBTree.h"

namespace ToolKit
{

  AABBTree::AABBTree() : root {nullNode}, nodeCapacity {32}, nodeCount {0} { Reset(); }

  AABBTree::~AABBTree()
  {
    nodes.clear();
    root      = nullNode;
    nodeCount = 0;
  }

  void AABBTree::Reset()
  {
    root      = nullNode;
    nodeCount = 0;
    nodes.resize(nodeCapacity);

    // Build a linked list for the free list.
    for (int32 i = 0; i < nodeCapacity - 1; ++i)
    {
      nodes[i].next   = i + 1;
      nodes[i].parent = i;
    }
    nodes[nodeCapacity - 1].next   = nullNode;
    nodes[nodeCapacity - 1].parent = nodeCapacity - 1;

    freeList                       = 0;
  }

  NodeProxy AABBTree::CreateNode(EntityWeakPtr entity, const BoundingBox& aabb)
  {
    NodeProxy newNode       = AllocateNode();

    // Fatten the aabb
    nodes[newNode].aabb.max = aabb.max + aabb_margin;
    nodes[newNode].aabb.min = aabb.min - aabb_margin;
    nodes[newNode].entity   = entity;
    nodes[newNode].parent   = nullNode;
    nodes[newNode].moved    = true;

    InsertLeaf(newNode);

    return newNode;
  }

  void AABBTree::Rebuild()
  {
    // Rebuild tree with bottom up approach

    std::vector<NodeProxy> leaves;
    leaves.resize(nodeCount);
    int32 count = 0;

    // Collect all leaves
    for (int32 i = 0; i < nodeCapacity; ++i)
    {
      // Already in the free list
      if (nodes[i].parent == i)
      {
        continue;
      }

      // Clean the leaf
      if (nodes[i].IsLeaf())
      {
        nodes[i].parent = nullNode;

        leaves[count++] = i;
      }
      else
      {
        // Free the internal node
        FreeNode(i);
      }
    }

    while (count > 1)
    {
      float minCost = TK_FLT_MAX;
      int32 minI    = -1;
      int32 minJ    = -1;

      // Find the best aabb pair
      for (int32 i = 0; i < count; ++i)
      {
        BoundingBox aabbI = nodes[leaves[i]].aabb;

        for (int32 j = i + 1; j < count; ++j)
        {
          BoundingBox aabbJ    = nodes[leaves[j]].aabb;

          BoundingBox combined = BoundingBox::Union(aabbI, aabbJ);
          float cost           = combined.SurfaceArea();

          if (cost < minCost)
          {
            minCost = cost;
            minI    = i;
            minJ    = j;
          }
        }
      }

      NodeProxy index1      = leaves[minI];
      NodeProxy index2      = leaves[minJ];
      Node* child1          = &nodes[index1];
      Node* child2          = &nodes[index2];

      // Create a parent(internal) node
      NodeProxy parentIndex = AllocateNode();
      Node* parent          = &nodes[parentIndex];

      parent->child1        = index1;
      parent->child2        = index2;
      parent->aabb          = BoundingBox::Union(child1->aabb, child2->aabb);
      parent->parent        = nullNode;

      child1->parent        = parentIndex;
      child2->parent        = parentIndex;

      leaves[minI]          = parentIndex;

      leaves[minJ]          = leaves[count - 1];
      --count;
    }

    root = leaves[0];
    leaves.clear();
  }

  NodeProxy AABBTree::AllocateNode()
  {
    if (freeList == nullNode)
    {
      assert(nodeCount == nodeCapacity);

      // Grow the node pool
      nodeCapacity += nodeCapacity / 2;
      nodes.resize(nodeCapacity);

      // Build a linked list for the free list.
      for (int32 i = nodeCount; i < nodeCapacity - 1; ++i)
      {
        nodes[i].next   = i + 1;
        nodes[i].parent = i;
      }
      nodes[nodeCapacity - 1].next   = nullNode;
      nodes[nodeCapacity - 1].parent = nodeCapacity - 1;

      freeList                       = nodeCount;
    }

    NodeProxy node     = freeList;
    freeList           = nodes[node].next;
    nodes[node].parent = nullNode;
    nodes[node].child1 = nullNode;
    nodes[node].child2 = nullNode;
    nodes[node].moved  = false;
    ++nodeCount;

    return node;
  }

  void AABBTree::FreeNode(NodeProxy node)
  {
    assert(0 <= node && node <= nodeCapacity);
    assert(0 < nodeCount);

    nodes[node].parent = node;
    nodes[node].next   = freeList;
    freeList           = node;

    --nodeCount;
  }

  void AABBTree::Rotate(NodeProxy node)
  {
    if (nodes[node].IsLeaf())
    {
      return;
    }

    NodeProxy child1   = nodes[node].child1;
    NodeProxy child2   = nodes[node].child2;

    float costDiffs[4] = {0.0f};

    if (nodes[child1].IsLeaf() == false)
    {
      float area1  = nodes[child1].aabb.SurfaceArea();
      costDiffs[0] = BoundingBox::Union(nodes[nodes[child1].child1].aabb, nodes[child2].aabb).SurfaceArea() - area1;
      costDiffs[1] = BoundingBox::Union(nodes[nodes[child1].child2].aabb, nodes[child2].aabb).SurfaceArea() - area1;
    }

    if (nodes[child2].IsLeaf() == false)
    {
      float area2  = nodes[child2].aabb.SurfaceArea();
      costDiffs[2] = BoundingBox::Union(nodes[nodes[child2].child1].aabb, nodes[child1].aabb).SurfaceArea() - area2;
      costDiffs[3] = BoundingBox::Union(nodes[nodes[child2].child2].aabb, nodes[child1].aabb).SurfaceArea() - area2;
    }

    int32 bestDiffIndex = 0;
    for (int32 i = 1; i < 4; ++i)
    {
      if (costDiffs[i] < costDiffs[bestDiffIndex])
      {
        bestDiffIndex = i;
      }
    }

    // Rotate only if it reduce the suface area
    if (costDiffs[bestDiffIndex] >= 0.0f)
    {
      return;
    }

    // printf("Tree rotation occurred: %d\n", bestDiffIndex);
    switch (bestDiffIndex)
    {
    case 0:
    {
      // Swap(child2, nodes[child1].child2);
      nodes[nodes[child1].child2].parent = node;
      nodes[node].child2                 = nodes[child1].child2;

      nodes[child1].child2               = child2;
      nodes[child2].parent               = child1;

      nodes[child1].aabb = BoundingBox::Union(nodes[nodes[child1].child1].aabb, nodes[nodes[child1].child2].aabb);
    }
    break;
    case 1:
    {
      // Swap(child2, nodes[child1].child1);
      nodes[nodes[child1].child1].parent = node;
      nodes[node].child2                 = nodes[child1].child1;

      nodes[child1].child1               = child2;
      nodes[child2].parent               = child1;

      nodes[child1].aabb = BoundingBox::Union(nodes[nodes[child1].child1].aabb, nodes[nodes[child1].child2].aabb);
    }
    break;
    case 2:
    {
      // Swap(child1, nodes[child2].child2);
      nodes[nodes[child2].child2].parent = node;
      nodes[node].child1                 = nodes[child2].child2;

      nodes[child2].child2               = child1;
      nodes[child1].parent               = child2;

      nodes[child2].aabb = BoundingBox::Union(nodes[nodes[child2].child1].aabb, nodes[nodes[child2].child2].aabb);
    }
    break;
    case 3:
    {
      // Swap(child1, nodes[child2].child1);
      nodes[nodes[child2].child1].parent = node;
      nodes[node].child1                 = nodes[child2].child1;

      nodes[child2].child1               = child1;
      nodes[child1].parent               = child2;

      nodes[child2].aabb = BoundingBox::Union(nodes[nodes[child2].child1].aabb, nodes[nodes[child2].child2].aabb);
    }
    break;
    }
  }

  NodeProxy AABBTree::InsertLeaf(NodeProxy leaf)
  {
    assert(0 <= leaf && leaf < nodeCapacity);
    assert(nodes[leaf].IsLeaf());

    if (root == nullNode)
    {
      root = leaf;
      return leaf;
    }

    BoundingBox aabb      = nodes[leaf].aabb;

    // Find the best sibling for the new leaf

    NodeProxy bestSibling = root;
    float bestCost        = BoundingBox::Union(nodes[root].aabb, aabb).SurfaceArea();

    // Candidate node with inherited cost
    struct Candidate
    {
      NodeProxy node;
      float inheritedCost;

      Candidate(NodeProxy node, float inheritedCost) : node(node), inheritedCost(inheritedCost) {}
    };

    std::queue<Candidate> stack;
    stack.emplace(root, 0.0f);

    while (stack.size() != 0)
    {
      NodeProxy current   = stack.back().node;
      float inheritedCost = stack.back().inheritedCost;
      stack.pop();

      BoundingBox combined = BoundingBox::Union(nodes[current].aabb, aabb);
      float directCost     = combined.SurfaceArea();

      float cost           = directCost + inheritedCost;
      if (cost < bestCost)
      {
        bestCost    = cost;
        bestSibling = current;
      }

      inheritedCost        += directCost - nodes[current].aabb.SurfaceArea();

      float lowerBoundCost  = aabb.SurfaceArea() + inheritedCost;
      if (lowerBoundCost < bestCost)
      {
        if (nodes[current].IsLeaf() == false)
        {
          stack.emplace(nodes[current].child1, inheritedCost);
          stack.emplace(nodes[current].child2, inheritedCost);
        }
      }
    }

    // Create a new parent
    NodeProxy oldParent     = nodes[bestSibling].parent;
    NodeProxy newParent     = AllocateNode();
    nodes[newParent].aabb   = BoundingBox::Union(aabb, nodes[bestSibling].aabb);
    nodes[newParent].parent = oldParent;
    nodes[newParent].entity.reset();

    // Connect new leaf and sibling to new parent
    nodes[newParent].child1   = leaf;
    nodes[newParent].child2   = bestSibling;
    nodes[leaf].parent        = newParent;
    nodes[bestSibling].parent = newParent;

    if (oldParent != nullNode)
    {
      if (nodes[oldParent].child1 == bestSibling)
      {
        nodes[oldParent].child1 = newParent;
      }
      else
      {
        nodes[oldParent].child2 = newParent;
      }
    }
    else
    {
      root = newParent;
    }

    // Walk back up the tree refitting ancestors' AABB and applying rotations
    NodeProxy ancestor = newParent;
    while (ancestor != nullNode)
    {
      NodeProxy child1     = nodes[ancestor].child1;
      NodeProxy child2     = nodes[ancestor].child2;

      nodes[ancestor].aabb = BoundingBox::Union(nodes[child1].aabb, nodes[child2].aabb);

      Rotate(ancestor);

      ancestor = nodes[ancestor].parent;
    }

    return leaf;
  }

} // namespace ToolKit
