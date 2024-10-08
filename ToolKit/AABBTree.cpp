/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "AABBTree.h"

#include "MathUtil.h"

namespace ToolKit
{

  AABBTree::AABBTree() : m_root {nullNode}, m_nodeCapacity {32}, m_nodeCount {0}, m_threadTreshold(1000) { Reset(); }

  AABBTree::~AABBTree()
  {
    m_nodes.clear();
    m_root      = nullNode;
    m_nodeCount = 0;
  }

  void AABBTree::Reset()
  {
    m_root      = nullNode;
    m_nodeCount = 0;
    m_nodes.resize(m_nodeCapacity);

    // Build a linked list for the free list.
    for (int32 i = 0; i < m_nodeCapacity - 1; ++i)
    {
      if (!m_nodes[i].entity.expired())
      {
        EntityPtr ntt            = m_nodes[i].entity.lock();
        ntt->m_aabbTreeNodeProxy = nullNode;
        m_nodes[i].entity.reset();
      }

      m_nodes[i].next   = i + 1;
      m_nodes[i].parent = i;
      m_nodes[i].leafs.clear();
    }
    m_nodes[m_nodeCapacity - 1].next   = nullNode;
    m_nodes[m_nodeCapacity - 1].parent = m_nodeCapacity - 1;

    m_freeList                         = 0;
  }

  AABBNodeProxy AABBTree::CreateNode(EntityWeakPtr entity, const BoundingBox& aabb)
  {
    AABBNodeProxy newNode = AllocateNode();

    if (EntityPtr ntt = entity.lock())
    {
      ntt->m_aabbTreeNodeProxy = newNode;
    }

    // Fatten the aabb
    m_nodes[newNode].aabb.max = aabb.max;
    m_nodes[newNode].aabb.min = aabb.min;
    m_nodes[newNode].entity   = entity;
    m_nodes[newNode].parent   = nullNode;
    m_nodes[newNode].moved    = true;

    // Insert a reference to self to create leafs struct properly in the tree.
    m_nodes[newNode].leafs.insert(newNode);

    InsertLeaf(newNode);

    return newNode;
  }

  void AABBTree::UpdateNode(AABBNodeProxy node)
  {
    assert(0 <= node && node < m_nodeCapacity);
    assert(m_nodes[node].IsLeaf());

    BoundingBox aabb = m_nodes[node].aabb;
    if (!m_nodes[node].entity.expired())
    {
      EntityPtr ntt = m_nodes[node].entity.lock();
      aabb          = ntt->GetBoundingBox(true);
    }

    aabb.max = aabb.max;
    aabb.min = aabb.min;

    RemoveLeaf(node);

    m_nodes[node].aabb = aabb;

    InsertLeaf(node);
  }

  void AABBTree::RemoveNode(AABBNodeProxy node)
  {
    assert(0 <= node && node < m_nodeCapacity);
    assert(m_nodes[node].IsLeaf());

    RemoveLeaf(node);
    FreeNode(node);
  }

  void AABBTree::Traverse(std::function<void(const AABBNode*)> callback) const
  {
    if (m_root == nullNode)
    {
      return;
    }

    std::deque<AABBNodeProxy> stack;
    stack.emplace_back(m_root);

    while (stack.size() != 0)
    {
      AABBNodeProxy current = stack.back();
      stack.pop_back();

      if (m_nodes[current].IsLeaf() == false)
      {
        stack.emplace_back(m_nodes[current].child1);
        stack.emplace_back(m_nodes[current].child2);
      }

      const AABBNode* node = &m_nodes[current];
      callback(node);
    }
  }

  void AABBTree::Rebuild()
  {
    // Rebuild tree with bottom up approach

    std::vector<AABBNodeProxy> leaves;
    leaves.resize(m_nodeCount);
    int32 count = 0;

    // Collect all leaves
    for (int32 i = 0; i < m_nodeCapacity; ++i)
    {
      // Already in the free list
      if (m_nodes[i].parent == i)
      {
        continue;
      }

      // Clean the leaf
      if (m_nodes[i].IsLeaf())
      {
        m_nodes[i].parent = nullNode;

        leaves[count++]   = i;
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
        BoundingBox aabbI = m_nodes[leaves[i]].aabb;

        for (int32 j = i + 1; j < count; ++j)
        {
          BoundingBox aabbJ    = m_nodes[leaves[j]].aabb;

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

      AABBNodeProxy index1      = leaves[minI];
      AABBNodeProxy index2      = leaves[minJ];
      AABBNode* child1          = &m_nodes[index1];
      AABBNode* child2          = &m_nodes[index2];

      // Create a parent(internal) node
      AABBNodeProxy parentIndex = AllocateNode();
      AABBNode* parent          = &m_nodes[parentIndex];

      parent->child1            = index1;
      parent->child2            = index2;
      parent->aabb              = BoundingBox::Union(child1->aabb, child2->aabb);
      parent->parent            = nullNode;

      child1->parent            = parentIndex;
      child2->parent            = parentIndex;

      leaves[minI]              = parentIndex;

      leaves[minJ]              = leaves[count - 1];
      --count;
    }

    if (!leaves.empty())
    {
      m_root = leaves[0];
      leaves.clear();
    }
  }

  const BoundingBox& AABBTree::GetRootBoundingBox() const
  {
    if (m_root != nullNode)
    {
      return m_nodes[m_root].aabb;
    }

    return infinitesimalBox;
  }

  AABBNodeProxy AABBTree::AllocateNode()
  {
    if (m_freeList == nullNode)
    {
      assert(m_nodeCount == m_nodeCapacity);

      // Grow the node pool
      m_nodeCapacity += m_nodeCapacity / 2;
      m_nodes.resize(m_nodeCapacity);

      // Build a linked list for the free list.
      for (int32 i = m_nodeCount; i < m_nodeCapacity - 1; ++i)
      {
        m_nodes[i].next   = i + 1;
        m_nodes[i].parent = i;
      }
      m_nodes[m_nodeCapacity - 1].parent = m_nodeCapacity - 1;
      m_nodes[m_nodeCapacity - 1].next   = nullNode;

      m_freeList                         = m_nodeCount;
    }

    AABBNodeProxy node   = m_freeList;
    m_freeList           = m_nodes[node].next;
    m_nodes[node].parent = nullNode;
    m_nodes[node].child1 = nullNode;
    m_nodes[node].child2 = nullNode;
    m_nodes[node].moved  = false;
    m_nodes[node].entity.reset();
    m_nodes[node].leafs.clear();
    ++m_nodeCount;

    return node;
  }

  /** Test tree against a frustum. */
  template TK_API EntityRawPtrArray AABBTree::VolumeQuery(const Frustum& frustum, bool threaded) const;

  /** Test tree against a box. */
  template TK_API EntityRawPtrArray AABBTree::VolumeQuery(const BoundingBox& box, bool threaded) const;

  template <typename VolumeType>
  EntityRawPtrArray AABBTree::VolumeQuery(const VolumeType& vol, bool threaded) const
  {

    EntityRawPtrArray entities;
    if (m_root == nullNode)
    {
      return entities;
    }

    EntityRawPtrArray entitiesInVolume;
    entitiesInVolume.resize(m_nodeCapacity);
    std::fill(entitiesInVolume.begin(), entitiesInVolume.end(), nullptr);

    m_maxThreadCount =
        m_nodeCount > m_threadTreshold && threaded ? GetWorkerManager()->GetThreadCount(WorkerManager::FramePool) : 0;

    std::atomic_int availableThreadCount(glm::max(0, m_maxThreadCount - 1));

    if constexpr (std::is_same_v<VolumeType, Frustum>)
    {
      VolumeQuery(entitiesInVolume,
                  availableThreadCount,
                  m_root,
                  [this, &vol](AABBNodeProxy root) -> IntersectResult
                  { return FrustumBoxIntersection(vol, m_nodes[root].aabb); });
    }
    else if constexpr (std::is_same_v<VolumeType, BoundingBox>)
    {
      VolumeQuery(entitiesInVolume,
                  availableThreadCount,
                  m_root,
                  [this, &vol](AABBNodeProxy root) -> IntersectResult
                  { return BoxBoxIntersection(vol, m_nodes[root].aabb); });
    }
    else
    {
      assert(0 && "Volume query is not implemented.");
      return entities;
    }

    // If there are threads in work, wait for them.
    HyperThreadSpinWait(availableThreadCount.load() < m_maxThreadCount);

    entities.reserve(m_nodeCapacity);

    for (int i = 0; i < (int) entitiesInVolume.size(); i++)
    {
      if (entitiesInVolume[i] != nullptr)
      {
        entities.push_back(entitiesInVolume[i]);
      }
    }

    return entities;
  }

  EntityPtr AABBTree::RayQuery(const Ray& ray, bool deep, float* t, const IDArray& ignoreList) const
  {
    if (m_root == nullNode)
    {
      return nullptr;
    }

    std::deque<AABBNodeProxy> stack;
    stack.emplace_back(m_root);

    float hitDist = TK_FLT_MAX;
    EntityPtr hitEntity;

    while (stack.size() != 0)
    {
      AABBNodeProxy current = stack.back();
      stack.pop_back();

      float intersecLen;
      if (RayBoxIntersection(ray, m_nodes[current].aabb, intersecLen))
      {
        if (m_nodes[current].IsLeaf())
        {
          EntityPtr candidate = m_nodes[current].entity.lock();
          if (!ignoreList.empty())
          {
            if (contains(ignoreList, candidate->GetIdVal()))
            {
              continue;
            }
          }

          if (deep)
          {
            float meshDist;
            if (RayEntityIntersection(ray, candidate, meshDist))
            {
              intersecLen = meshDist;
            }
            else
            {
              intersecLen = TK_FLT_MAX;
            }
          }

          if (intersecLen < hitDist)
          {
            hitDist   = intersecLen;
            hitEntity = candidate;
          }
        }
        else
        {
          stack.emplace_back(m_nodes[current].child1);
          stack.emplace_back(m_nodes[current].child2);
        }
      }
    }

    if (t != nullptr)
    {
      *t = hitDist;
    }

    return hitEntity;
  }

  void AABBTree::GetDebugBoundingBoxes(EntityPtrArray& boundingBoxes) const
  {
    Traverse([&](const AABBNode* node) -> void
             { boundingBoxes.push_back(CreateBoundingBoxDebugObject(node->aabb, ZERO, 1.0f)); });
  }

  void AABBTree::FreeNode(AABBNodeProxy node)
  {
    assert(0 <= node && node <= m_nodeCapacity);
    assert(0 < m_nodeCount);

    if (EntityPtr ntt = m_nodes[node].entity.lock())
    {
      ntt->m_aabbTreeNodeProxy = nullNode;
    }

    m_nodes[node].parent = node;
    m_nodes[node].next   = m_freeList;
    m_nodes[node].entity.reset();
    m_nodes[node].leafs.clear();
    m_freeList = node;

    --m_nodeCount;
  }

  void AABBTree::Rotate(AABBNodeProxy node)
  {
    if (m_nodes[node].IsLeaf())
    {
      return;
    }

    AABBNodeProxy child1 = m_nodes[node].child1;
    AABBNodeProxy child2 = m_nodes[node].child2;

    float costDiffs[4]   = {0.0f};

    if (m_nodes[child1].IsLeaf() == false)
    {
      float area1 = m_nodes[child1].aabb.SurfaceArea();
      costDiffs[0] =
          BoundingBox::Union(m_nodes[m_nodes[child1].child1].aabb, m_nodes[child2].aabb).SurfaceArea() - area1;
      costDiffs[1] =
          BoundingBox::Union(m_nodes[m_nodes[child1].child2].aabb, m_nodes[child2].aabb).SurfaceArea() - area1;
    }

    if (m_nodes[child2].IsLeaf() == false)
    {
      float area2 = m_nodes[child2].aabb.SurfaceArea();
      costDiffs[2] =
          BoundingBox::Union(m_nodes[m_nodes[child2].child1].aabb, m_nodes[child1].aabb).SurfaceArea() - area2;
      costDiffs[3] =
          BoundingBox::Union(m_nodes[m_nodes[child2].child2].aabb, m_nodes[child1].aabb).SurfaceArea() - area2;
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

    // n0 does not invalidate its parent leaf cache, n1 does due to depth difference.
    // n0 is at a lower depth, moving it a higher depth does not alter n's leafs.
    auto leafCacheUpdate = [&](AABBNodeProxy n0, AABBNodeProxy n1) -> void
    {
      AABBNodeProxy n0sibling = m_nodes[n1].parent;
      assert(n0sibling != nullNode);

      for (AABBNodeProxy leaf : m_nodes[n1].leafs)
      {
        m_nodes[n0sibling].leafs.erase(leaf);
      }

      for (AABBNodeProxy leaf : m_nodes[n0].leafs)
      {
        m_nodes[n0sibling].leafs.insert(leaf);
      }
    };

    // printf("Tree rotation occurred: %d\n", bestDiffIndex);
    switch (bestDiffIndex)
    {
    case 0:
    {
      // Swap(child2, nodes[child1].child2);
      leafCacheUpdate(child2, m_nodes[child1].child2);

      m_nodes[m_nodes[child1].child2].parent = node;
      m_nodes[node].child2                   = m_nodes[child1].child2;

      m_nodes[child1].child2                 = child2;
      m_nodes[child2].parent                 = child1;

      m_nodes[child1].aabb =
          BoundingBox::Union(m_nodes[m_nodes[child1].child1].aabb, m_nodes[m_nodes[child1].child2].aabb);
    }
    break;
    case 1:
    {
      // Swap(child2, nodes[child1].child1);
      leafCacheUpdate(child2, m_nodes[child1].child1);

      m_nodes[m_nodes[child1].child1].parent = node;
      m_nodes[node].child2                   = m_nodes[child1].child1;

      m_nodes[child1].child1                 = child2;
      m_nodes[child2].parent                 = child1;

      m_nodes[child1].aabb =
          BoundingBox::Union(m_nodes[m_nodes[child1].child1].aabb, m_nodes[m_nodes[child1].child2].aabb);
    }
    break;
    case 2:
    {
      // Swap(child1, nodes[child2].child2);
      leafCacheUpdate(child1, m_nodes[child2].child2);

      m_nodes[m_nodes[child2].child2].parent = node;
      m_nodes[node].child1                   = m_nodes[child2].child2;

      m_nodes[child2].child2                 = child1;
      m_nodes[child1].parent                 = child2;
      m_nodes[child2].aabb =
          BoundingBox::Union(m_nodes[m_nodes[child2].child1].aabb, m_nodes[m_nodes[child2].child2].aabb);
    }
    break;
    case 3:
    {
      // Swap(child1, nodes[child2].child1);
      leafCacheUpdate(child1, m_nodes[child2].child1);

      m_nodes[m_nodes[child2].child1].parent = node;
      m_nodes[node].child1                   = m_nodes[child2].child1;

      m_nodes[child2].child1                 = child1;
      m_nodes[child1].parent                 = child2;

      m_nodes[child2].aabb =
          BoundingBox::Union(m_nodes[m_nodes[child2].child1].aabb, m_nodes[m_nodes[child2].child2].aabb);
    }
    break;
    }
  }

  void AABBTree::VolumeQuery(EntityRawPtrArray& result,
                             std::atomic_int& threadCount,
                             AABBNodeProxy root,
                             std::function<enum class IntersectResult(AABBNodeProxy)> queryFn) const
  {
    std::deque<AABBNodeProxy> stack;
    stack.emplace_back(root);

    while (stack.size() != 0)
    {
      AABBNodeProxy current = stack.back();
      stack.pop_back();

      IntersectResult intResult = queryFn(current);

      if (intResult == IntersectResult::Intersect)
      {
        // Volume is partially inside, check all internal volumes.
        if (m_nodes[current].IsLeaf())
        {
          if (!m_nodes[current].entity.expired())
          {
            result[current] = m_nodes[current].entity.lock().get();
          }
        }
        else
        {
          auto parallelProcessFn = [&](AABBNodeProxy node) -> void
          {
            if (node == nullNode)
            {
              return;
            }

            int currentCount = threadCount.load();
            if (currentCount > 0)
            {
              if (threadCount.compare_exchange_strong(currentCount, currentCount - 1))
              {
                TKAsyncTask(WorkerManager::FramePool,
                            [this, &result, &threadCount, node, queryFn]() -> void
                            { VolumeQuery(result, threadCount, node, queryFn); });
                return;
              }
            }

            stack.emplace_back(node);
          };

          parallelProcessFn(m_nodes[current].child1);
          parallelProcessFn(m_nodes[current].child2);
        }
      }
      else if (intResult == IntersectResult::Inside)
      {
        // Volume is fully inside, get all entities from cache.
        for (AABBNodeProxy leaf : m_nodes[current].leafs)
        {
          if (!m_nodes[leaf].entity.expired())
          {
            result[leaf] = m_nodes[leaf].entity.lock().get();
          }
        }
      }
    }

    threadCount.fetch_add(1);
  }

  AABBNodeProxy AABBTree::InsertLeaf(AABBNodeProxy leaf)
  {
    assert(0 <= leaf && leaf < m_nodeCapacity);
    assert(m_nodes[leaf].IsLeaf());

    if (m_root == nullNode)
    {
      m_root = leaf;
      return leaf;
    }

    BoundingBox aabb          = m_nodes[leaf].aabb;

    // Find the best sibling for the new leaf

    AABBNodeProxy bestSibling = m_root;
    float bestCost            = BoundingBox::Union(m_nodes[m_root].aabb, aabb).SurfaceArea();

    // Candidate node with inherited cost
    struct Candidate
    {
      AABBNodeProxy node;
      float inheritedCost;

      Candidate(AABBNodeProxy node, float inheritedCost) : node(node), inheritedCost(inheritedCost) {}
    };

    std::deque<Candidate> stack;
    stack.emplace_back(m_root, 0.0f);

    while (stack.size() != 0)
    {
      AABBNodeProxy current = stack.back().node;
      float inheritedCost   = stack.back().inheritedCost;
      stack.pop_back();

      BoundingBox combined = BoundingBox::Union(m_nodes[current].aabb, aabb);
      float directCost     = combined.SurfaceArea();

      float cost           = directCost + inheritedCost;
      if (cost < bestCost)
      {
        bestCost    = cost;
        bestSibling = current;
      }

      inheritedCost        += directCost - m_nodes[current].aabb.SurfaceArea();

      float lowerBoundCost  = aabb.SurfaceArea() + inheritedCost;
      if (lowerBoundCost < bestCost)
      {
        if (m_nodes[current].IsLeaf() == false)
        {
          stack.emplace_back(m_nodes[current].child1, inheritedCost);
          stack.emplace_back(m_nodes[current].child2, inheritedCost);
        }
      }
    }

    // Create a new parent
    AABBNodeProxy oldParent     = m_nodes[bestSibling].parent;
    AABBNodeProxy newParent     = AllocateNode();
    m_nodes[newParent].aabb     = BoundingBox::Union(aabb, m_nodes[bestSibling].aabb);
    m_nodes[newParent].parent   = oldParent;

    // Connect new leaf and sibling to new parent
    m_nodes[newParent].child1   = leaf;
    m_nodes[newParent].child2   = bestSibling;
    m_nodes[leaf].parent        = newParent;
    m_nodes[bestSibling].parent = newParent;

    if (oldParent != nullNode)
    {
      if (m_nodes[oldParent].child1 == bestSibling)
      {
        m_nodes[oldParent].child1 = newParent;
      }
      else
      {
        m_nodes[oldParent].child2 = newParent;
      }
    }
    else
    {
      m_root = newParent;
    }

    // Construct new parent's leaf cache.
    if (m_nodes[bestSibling].IsLeaf())
    {
      m_nodes[newParent].leafs.insert(bestSibling);
    }
    else
    {
      for (AABBNodeProxy sibLeaf : m_nodes[bestSibling].leafs)
      {
        m_nodes[newParent].leafs.insert(sibLeaf);
      }
    }

    // Walk back up the tree refitting ancestors' AABB and applying rotations
    AABBNodeProxy ancestor = newParent;
    while (ancestor != nullNode)
    {
      m_nodes[ancestor].leafs.insert(leaf); // Insert to all ancestors.

      AABBNodeProxy child1   = m_nodes[ancestor].child1;
      AABBNodeProxy child2   = m_nodes[ancestor].child2;

      m_nodes[ancestor].aabb = BoundingBox::Union(m_nodes[child1].aabb, m_nodes[child2].aabb);

      Rotate(ancestor);

      ancestor = m_nodes[ancestor].parent;
    }

    return leaf;
  }

  void AABBTree::RemoveLeaf(AABBNodeProxy leaf)
  {
    assert(0 <= leaf && leaf < m_nodeCapacity);
    assert(m_nodes[leaf].IsLeaf());

    AABBNodeProxy parent = m_nodes[leaf].parent;
    if (parent == nullNode) // node is root
    {
      assert(m_root == leaf);
      m_root = nullNode;
      return;
    }

    AABBNodeProxy grandParent = m_nodes[parent].parent;
    AABBNodeProxy sibling;
    if (m_nodes[parent].child1 == leaf)
    {
      sibling = m_nodes[parent].child2;
    }
    else
    {
      sibling = m_nodes[parent].child1;
    }

    FreeNode(parent);

    if (grandParent != nullNode) // node has grandparent
    {
      m_nodes[sibling].parent = grandParent;

      if (m_nodes[grandParent].child1 == parent)
      {
        m_nodes[grandParent].child1 = sibling;
      }
      else
      {
        m_nodes[grandParent].child2 = sibling;
      }

      AABBNodeProxy ancestor = grandParent;
      while (ancestor != nullNode)
      {
        m_nodes[ancestor].leafs.erase(leaf); // Remove from all ancestors.

        AABBNodeProxy child1   = m_nodes[ancestor].child1;
        AABBNodeProxy child2   = m_nodes[ancestor].child2;

        m_nodes[ancestor].aabb = BoundingBox::Union(m_nodes[child1].aabb, m_nodes[child2].aabb);

        Rotate(ancestor);

        ancestor = m_nodes[ancestor].parent;
      }
    }
    else // node has no grandparent
    {
      m_root                  = sibling;
      m_nodes[sibling].parent = nullNode;
    }
  }

} // namespace ToolKit
