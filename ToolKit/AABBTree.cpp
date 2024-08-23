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

} // namespace ToolKit
