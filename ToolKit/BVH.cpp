#include "BVH.h"

#include "MathUtil.h"
#include "Renderer.h"
#include "Scene.h"

namespace ToolKit
{
  inline bool IsBVHEntity(const EntityPtr& ntt) { return !ntt->IsA<Sky>(); }

  BVH::BVH(Scene* scene)
  {
    m_bvhTree = new BVHTree(this);
    m_scene   = scene;
  }

  BVH::~BVH() { SafeDel(m_bvhTree); }

  void BVH::SetParameters(const EngineSettings::PostProcessingSettings& settings)
  {
    m_bvhTree->m_maxEntityCountPerBVHNode = settings.maxEntityPerBVHNode;
    m_bvhTree->m_minBBSize                = settings.minBVHNodeSize;
  }

  bool BVH::ReBuild()
  {
    // Get new parameters
    SetParameters(GetEngineSettings().PostProcessing);

    // Clean scene entities
    for (EntityPtr ntt : m_scene->GetEntities())
    {
      ntt->m_bvhNodes.clear();
    }

    // Clean bvh tree
    Clean();

    // Rebuild
    for (EntityPtr ntt : m_scene->GetEntities())
    {
      if (IsBVHEntity(ntt))
      {
        if (m_bvhTree->Add(ntt))
        {
          return true;
        }
      }
    }

    return false;
  }

  void BVH::Clean()
  {
    for (EntityPtr ntt : m_scene->GetEntities())
    {
      ntt->m_bvhNodes.clear();
    }

    m_bvhTree->Clean();
  }

  void BVH::AddEntity(const EntityPtr& entity)
  {
    if (!entity->_addingToBVH && IsBVHEntity(entity))
    {
      entity->_addingToBVH = true;
      m_entitiesToAdd.push_back(entity);
    }
  }

  void BVH::RemoveEntity(const EntityPtr& entity)
  {
    if (IsBVHEntity(entity))
    {
      m_entitiesToRemove.push_back(entity);
    }
  }

  void BVH::UpdateEntity(const EntityPtr& entity)
  {
    if (IsBVHEntity(entity))
    {
      m_entitiesToUpdate.push_back(entity);
    }
  }

  void BVH::Update()
  {
    // Removed entities
    for (EntityPtr& entity : m_entitiesToRemove)
    {
      m_bvhTree->Remove(entity);
    }
    m_entitiesToRemove.clear();

    // Delete nodes that should be deleted
    for (int i = 0; i < m_bvhTree->m_nodesToDelete.size(); ++i)
    {
      BVHNode* node = m_bvhTree->m_nodesToDelete[i];
      SafeDel(node);
    }
    m_bvhTree->m_nodesToDelete.clear();

    for (EntityPtr& entity : m_entitiesToUpdate)
    {
      m_bvhTree->Remove(entity);
      AddEntity(entity);
    }
    m_entitiesToUpdate.clear();

    for (int i = 0; i < m_bvhTree->m_nodesToDelete.size(); ++i)
    {
      BVHNode* node = m_bvhTree->m_nodesToDelete[i];
      SafeDel(node);
    }
    m_bvhTree->m_nodesToDelete.clear();

    for (EntityPtr& entity : m_entitiesToAdd)
    {
      if (m_bvhTree->Add(entity))
      {
        while (ReBuild()) {}
        break;
      }
    }
    m_entitiesToAdd.clear();
  }

  void BVH::PickObject(const Ray& ray, Scene::PickData& pickData, const IDArray& ignoreList, float& closestDistance)
  {
    m_bvhTree->m_nextNodes.clear();
    m_bvhTree->m_nextNodes.push_front(m_bvhTree->m_root);
    while (!m_bvhTree->m_nextNodes.empty())
    {
      BVHNode* currentNode = m_bvhTree->m_nextNodes.front();
      m_bvhTree->m_nextNodes.pop_front();

      float dist;
      if (RayBoxIntersection(ray, currentNode->m_aabb, dist))
      {
        if (currentNode->Leaf())
        {
          for (EntityPtr ntt : currentNode->m_entites)
          {
            if (contains(ignoreList, ntt->GetIdVal()))
            {
              continue;
            }

            float dist = TK_FLT_MAX;
            if (RayEntityIntersection(ray, ntt, dist))
            {
              if (dist < closestDistance && dist > 0.0f)
              {
                pickData.entity  = ntt;
                pickData.pickPos = ray.position + ray.direction * dist;
                closestDistance  = dist;
              }
            }
          }
        }
        else
        {
          m_bvhTree->m_nextNodes.push_front(currentNode->m_left);
          m_bvhTree->m_nextNodes.push_front(currentNode->m_right);
        }
      }
    }
  }

  void BVH::PickObject(const Frustum& frustum,
                       Scene::PickDataArray& pickedObjects,
                       const IDArray& ignoreList,
                       const EntityPtrArray& extraList,
                       bool pickPartiallyInside)
  {
    m_bvhTree->m_nextNodes.clear();
    m_bvhTree->m_nextNodes.push_front(m_bvhTree->m_root);
    while (!m_bvhTree->m_nextNodes.empty())
    {
      BVHNode* currentNode = m_bvhTree->m_nextNodes.front();
      m_bvhTree->m_nextNodes.pop_front();

      // If the nodes parent already is inside of the frustum, no need to check intersection
      if (currentNode->m_insideFrustum)
      {
        if (currentNode->Leaf())
        {
          for (EntityPtr ntt : currentNode->m_entites)
          {
            if (contains(ignoreList, ntt->GetIdVal()))
            {
              continue;
            }

            const BoundingBox box = ntt->GetBoundingBox(true);
            Scene::PickData pd;
            pd.pickPos = (box.max + box.min) * 0.5f;
            pd.entity  = ntt;
            pickedObjects.push_back(pd);
          }
        }
        else
        {
          currentNode->m_left->m_insideFrustum  = true;
          currentNode->m_right->m_insideFrustum = true;
          m_bvhTree->m_nextNodes.push_back(currentNode->m_left);
          m_bvhTree->m_nextNodes.push_back(currentNode->m_right);
        }

        currentNode->m_insideFrustum = false;
        continue;
      }

      IntersectResult res = FrustumBoxIntersection(frustum, currentNode->m_aabb);
      if (res == IntersectResult::Inside)
      {
        // This node and all children nodes of this node do not need to check intersection again
        currentNode->m_insideFrustum = true;
        m_bvhTree->m_nextNodes.push_back(currentNode);
      }
      else if (res == IntersectResult::Intersect)
      {
        if (currentNode->Leaf())
        {
          for (EntityPtr ntt : currentNode->m_entites)
          {
            if (contains(ignoreList, ntt->GetIdVal()))
            {
              continue;
            }

            const BoundingBox box = ntt->GetBoundingBox(true);
            IntersectResult res   = FrustumBoxIntersection(frustum, box);

            if (res != IntersectResult::Outside)
            {
              Scene::PickData pd;
              pd.pickPos = (box.max + box.min) * 0.5f;
              pd.entity  = ntt;

              if (res == IntersectResult::Inside)
              {
                pickedObjects.push_back(pd);
              }
              else if (pickPartiallyInside)
              {
                pickedObjects.push_back(pd);
              }
            }
          }
        }
        else
        {
          m_bvhTree->m_nextNodes.push_front(currentNode->m_left);
          m_bvhTree->m_nextNodes.push_front(currentNode->m_right);
        }
      }
    }
  }

  void BVH::GetDebugBVHBoxes(EntityPtrArray& boxes)
  {
    std::queue<BVHNode*> nextNodes;
    nextNodes.push(m_bvhTree->m_root);
    while (nextNodes.size() > 0)
    {
      BVHNode* bvhNode = nextNodes.front();
      nextNodes.pop();

      if (!bvhNode->Leaf())
      {
        nextNodes.push(bvhNode->m_left);
        nextNodes.push(bvhNode->m_right);
      }
      else
      {
        boxes.push_back(Cast<Entity>(CreateBoundingBoxDebugObject(bvhNode->m_aabb, Vec3(1.0f, 0.4f, 0.1f), 0.75f)));
      }
    }
  }

  void BVH::SanityCheck()
  {
    // check if any entity holds a node that is not leaf
    for (EntityPtr ntt : m_scene->GetEntities())
    {
      for (BVHNode* node : ntt->m_bvhNodes)
      {
        assert(node->Leaf() && "Entities should not hold non-leaf bvh nodes");
      }
    }
  }

  BVHTree::BVHTree(BVH* owner)
  {
    m_bvh                      = owner;
    m_root                     = new BVHNode();

    m_maxEntityCountPerBVHNode = 5;
    m_minBBSize                = 10.0f;
  }

  BVHTree::~BVHTree()
  {
    Clean();
    SafeDel(m_root);
  }

  bool BVHTree::Add(EntityPtr& entity)
  {
    entity->_addingToBVH = false;

    BoundingBox entityAABB;
    BoundingSphere lightSphere;
    BoundingBox lightBBox;

    /*
     * 0 : entity
     * 1 : spot light
     * 2 : point light
     */
    int entityType = 0;

    // Check if the root bounding box should get bigger. If this is the case, we need to recalculate the entire
    // hierarchy.

    if (entity->IsA<Light>())
    {
      // Light does cause increasing
      if (SpotLight* spot = entity->As<SpotLight>())
      {
        entityType = 1;

        spot->UpdateShadowCamera();
        lightBBox = spot->m_boundingBoxCache;

        if (!BoxBoxIntersection(lightBBox, m_root->m_aabb))
        {
          m_root->m_aabb.UpdateBoundary(entity->m_node->GetTranslation());
          return true;
        }
      }
      else if (PointLight* point = entity->As<PointLight>())
      {
        entityType = 2;

        point->UpdateShadowCamera();
        lightSphere = point->m_boundingSphereCache;

        if (!SphereBoxIntersection(lightSphere, m_root->m_aabb))
        {
          m_root->m_aabb.UpdateBoundary(entity->m_node->GetTranslation());
          return true;
        }
      }
    }
    else
    {
      entityAABB = entity->GetBoundingBox(true);
      if (!BoxInsideBox(entityAABB, m_root->m_aabb))
      {
        m_root->m_aabb.UpdateBoundary(entityAABB);

        return true;
      }
    }

    // Add entity to the bvh tree

    std::queue<BVHNode*> nextNodes;
    nextNodes.push(m_root);
    while (nextNodes.size() > 0)
    {
      BVHNode* node = nextNodes.front();
      nextNodes.pop();

      if (node->m_waitingForDeletion)
      {
        continue;
      }

      if (!node->Leaf())
      {
        // intermediate node

        if (entityType == 1) // spot light
        {
          if (BoxBoxIntersection(lightBBox, node->m_left->m_aabb))
          {
            nextNodes.push(node->m_left);
          }
          if (BoxBoxIntersection(lightBBox, node->m_right->m_aabb))
          {
            nextNodes.push(node->m_right);
          }
        }
        else if (entityType == 2) // point light
        {
          if (SphereBoxIntersection(lightSphere, node->m_left->m_aabb))
          {
            nextNodes.push(node->m_left);
          }
          if (SphereBoxIntersection(lightSphere, node->m_right->m_aabb))
          {
            nextNodes.push(node->m_right);
          }
        }
        else // entity
        {
          if (BoxBoxIntersection(entityAABB, node->m_left->m_aabb))
          {
            nextNodes.push(node->m_left);
          }
          if (BoxBoxIntersection(entityAABB, node->m_right->m_aabb))
          {
            nextNodes.push(node->m_right);
          }
        }
      }
      else
      {
        // leaf node

        if (entityType == 1 || entityType == 2) // light
        {
          node->m_lights.push_back(entity);
        }
        else // entity
        {
          node->m_entites.push_back(entity);
        }
        entity->m_bvhNodes.push_back(node);

        UpdateLeaf(node);
      }
    }

    return false;
  }

  void BVHTree::Remove(EntityPtr& entity)
  {
    for (BVHNode* bvhNode : entity->m_bvhNodes)
    {
      uint i = 0;
      for (EntityPtr ntt : bvhNode->m_entites)
      {
        if (ntt == entity)
        {
          bvhNode->m_entites.erase(bvhNode->m_entites.begin() + i);
          break;
        }
        ++i;
      }
      i = 0;
      for (EntityPtr ntt : bvhNode->m_lights)
      {
        if (ntt == entity)
        {
          bvhNode->m_lights.erase(bvhNode->m_lights.begin() + i);
          break;
        }
        ++i;
      }
    }

    for (BVHNode* bvhNode : entity->m_bvhNodes)
    {
      if (bvhNode->m_waitingForDeletion)
      {
        continue;
      }
      UpdateLeaf(bvhNode);
    }

    entity->m_bvhNodes.clear();
  }

  void BVHTree::Clean()
  {
    // Clean bvh tree
    std::queue<BVHNode*> nextNodes;
    nextNodes.push(m_root);
    while (nextNodes.size() > 0)
    {
      BVHNode* bvhNode = nextNodes.front();
      nextNodes.pop();

      if (bvhNode->m_left != nullptr)
      {
        nextNodes.push(bvhNode->m_left);
      }

      if (bvhNode->m_right != nullptr)
      {
        nextNodes.push(bvhNode->m_right);
      }

      if (m_root == bvhNode)
      {
        m_root->m_left  = nullptr;
        m_root->m_right = nullptr;
        m_root->m_entites.clear();
        m_root->m_lights.clear();
      }
      else
      {
        SafeDel(bvhNode);
      }
    }
  }

  void SplitBoundingBox(BoundingBox bb, AxisLabel axis, BoundingBox& outLeft, BoundingBox& outRight)
  {
    switch (axis)
    {
    case AxisLabel::X:
    {
      float midX = (bb.min.x + bb.max.x) / 2.0f;
      BoundingBox left;
      left.min   = bb.min;
      left.max.x = midX;
      left.max.y = bb.max.y;
      left.max.z = bb.max.z;
      outLeft    = left;

      BoundingBox right;
      right.min.x = midX;
      right.min.y = bb.min.y;
      right.min.z = bb.min.z;
      right.max   = bb.max;
      outRight    = right;
      break;
    }
    case AxisLabel::Y:
    {
      float midY = (bb.min.y + bb.max.y) / 2.0f;

      BoundingBox left;
      left.min   = bb.min;
      left.max.x = bb.max.x;
      left.max.y = midY;
      left.max.z = bb.max.z;
      outLeft    = left;

      BoundingBox right;
      right.min.x = bb.min.x;
      right.min.y = midY;
      right.min.z = bb.min.z;
      right.max   = bb.max;
      outRight    = right;
      break;
    }
    case AxisLabel::Z:
    {
      float midZ = (bb.min.z + bb.max.z) / 2.0f;

      BoundingBox left;
      left.min   = bb.min;
      left.max.x = bb.max.x;
      left.max.y = bb.max.y;
      left.max.z = midZ;
      outLeft    = left;

      BoundingBox right;
      right.min.x = bb.min.x;
      right.min.y = bb.min.y;
      right.min.z = midZ;
      right.max   = bb.max;
      outRight    = right;
      break;
    }
    default:
      assert(false && "Invalid Axis Label");
      break;
    }
  }

  AxisLabel GetLongestAxis(const BoundingBox& bb)
  {
    float x = bb.max.x - bb.min.x;
    float y = bb.max.y - bb.min.y;
    float z = bb.max.z - bb.min.z;

    if (x >= y && x >= z)
    {
      return AxisLabel::X;
    }
    else if (y >= x && y >= z)
    {
      return AxisLabel::Y;
    }
    else if (z >= x && z >= y)
    {
      return AxisLabel::Z;
    }
    else
    {
      return AxisLabel::X;
    }
  }

  void BVHTree::UpdateLeaf(BVHNode* node)
  {
    if (!node->Leaf())
    {
      assert(false && "Calling UpdateLeaf() on a non-leaf bvh node. Needs to be fixed!");
      return;
    }

    const size_t entityCount = node->m_entites.size();
    if (entityCount > m_maxEntityCountPerBVHNode)
    {
      // split this node if entity number is big

      BVHNode* left           = new BVHNode();
      BVHNode* right          = new BVHNode();
      left->depth             = node->depth + 1;
      right->depth            = node->depth + 1;

      const AxisLabel maxAxis = GetLongestAxis(node->m_aabb);
      SplitBoundingBox(node->m_aabb, maxAxis, left->m_aabb, right->m_aabb);

      if (left->m_aabb.GetWidth() < m_minBBSize || left->m_aabb.GetHeight() < m_minBBSize ||
          right->m_aabb.GetWidth() < m_minBBSize || right->m_aabb.GetHeight() < m_minBBSize)
      {
        SafeDel(left);
        SafeDel(right);
      }
      else
      {
        // divide entities into leaf nodes
        for (EntityPtr& entity : node->m_entites)
        {
          const BoundingBox& entityAABB = entity->GetBoundingBox(true);
          if (BoxBoxIntersection(left->m_aabb, entityAABB))
          {
            entity->m_bvhNodes.push_back(left);
            left->m_entites.push_back(entity);
          }
          if (BoxBoxIntersection(right->m_aabb, entityAABB))
          {
            entity->m_bvhNodes.push_back(right);
            right->m_entites.push_back(entity);
          }
        }
        for (EntityPtr lightEntity : node->m_lights)
        {
          if (SpotLight* spot = lightEntity->As<SpotLight>())
          {
            if (IntersectResult::Outside != FrustumBoxIntersection(spot->m_frustumCache, left->m_aabb))
            {
              lightEntity->m_bvhNodes.push_back(left);
              left->m_lights.push_back(lightEntity);
            }
            if (IntersectResult::Outside != FrustumBoxIntersection(spot->m_frustumCache, right->m_aabb))
            {
              lightEntity->m_bvhNodes.push_back(right);
              right->m_lights.push_back(lightEntity);
            }
          }
          else if (PointLight* point = lightEntity->As<PointLight>())
          {
            if (SphereBoxIntersection(point->m_boundingSphereCache, left->m_aabb))
            {
              lightEntity->m_bvhNodes.push_back(left);
              left->m_lights.push_back(lightEntity);
            }
            if (SphereBoxIntersection(point->m_boundingSphereCache, right->m_aabb))
            {
              lightEntity->m_bvhNodes.push_back(right);
              right->m_lights.push_back(lightEntity);
            }
          }
        }

        left->m_parent  = node;
        right->m_parent = node;
        node->m_left    = left;
        node->m_right   = right;

        // Remove this node from the entity
        for (EntityPtr ntt : node->m_entites)
        {
          uint i = 0;
          for (BVHNode* nttNode : ntt->m_bvhNodes)
          {
            if (nttNode == node)
            {
              ntt->m_bvhNodes.erase(ntt->m_bvhNodes.begin() + i);
              break;
            }
            ++i;
          }
        }
        for (EntityPtr ntt : node->m_lights)
        {
          uint i = 0;
          for (BVHNode* nttNode : ntt->m_bvhNodes)
          {
            if (nttNode == node)
            {
              ntt->m_bvhNodes.erase(ntt->m_bvhNodes.begin() + i);
              break;
            }
            ++i;
          }
        }
        node->m_entites.clear();
        node->m_lights.clear();

        UpdateLeaf(left);
        if (!right->m_waitingForDeletion)
        {
          UpdateLeaf(right);
        }
      }
    }
    else if (entityCount <= m_maxEntityCountPerBVHNode && node != m_root)
    {
      BVHNode* parentNode = node->m_parent;
      BVHNode* rightNode  = node->m_parent->m_right;
      BVHNode* leftNode   = node->m_parent->m_left;

      bool conjunct       = false;
      if (leftNode == node)
      {
        if (rightNode->Leaf() && rightNode->m_entites.size() + entityCount <= m_maxEntityCountPerBVHNode)
        {
          conjunct = true;
        }
      }
      else // if (rightNode == node)
      {
        if (leftNode->Leaf() && leftNode->m_entites.size() + entityCount <= m_maxEntityCountPerBVHNode)
        {
          conjunct = true;
        }
      }

      if (conjunct)
      {
        // conjunct two child nodes into parent
        parentNode->m_left  = nullptr;
        parentNode->m_right = nullptr;
        parentNode->m_entites.insert(parentNode->m_entites.end(),
                                     leftNode->m_entites.begin(),
                                     leftNode->m_entites.end());
        parentNode->m_entites.insert(parentNode->m_entites.end(),
                                     rightNode->m_entites.begin(),
                                     rightNode->m_entites.end());
        parentNode->m_lights.insert(parentNode->m_lights.end(), leftNode->m_lights.begin(), leftNode->m_lights.end());
        parentNode->m_lights.insert(parentNode->m_lights.end(), rightNode->m_lights.begin(), rightNode->m_lights.end());

        // remove duplicate entities
        RemoveDuplicates(parentNode->m_entites);
        RemoveDuplicates(parentNode->m_lights);

        rightNode->m_waitingForDeletion = true;
        leftNode->m_waitingForDeletion  = true;
        m_nodesToDelete.push_back(rightNode);
        m_nodesToDelete.push_back(leftNode);

        // Tell entities about the new conjuncted node
        for (EntityPtr ntt : parentNode->m_entites)
        {
          ntt->m_bvhNodes.push_back(parentNode);
        }
        for (EntityPtr ntt : parentNode->m_lights)
        {
          ntt->m_bvhNodes.push_back(parentNode);
        }

        UpdateLeaf(parentNode);
      }
    }
  }

  BVHNode::BVHNode() { m_aabb = BoundingBox(Vec3(-100.0f), Vec3(100.0f)); }

  BVHNode::~BVHNode()
  { // Remove this node from all entities
    for (EntityPtr ntt : m_entites)
    {
      uint i = 0;
      for (BVHNode* bvhNode : ntt->m_bvhNodes)
      {
        if (bvhNode == this)
        {
          ntt->m_bvhNodes.erase(ntt->m_bvhNodes.begin() + i);
          break;
        }
        i++;
      }
    }
    m_entites.clear();

    for (EntityPtr ntt : m_lights)
    {
      uint i = 0;
      for (BVHNode* bvhNode : ntt->m_bvhNodes)
      {
        if (bvhNode == this)
        {
          ntt->m_bvhNodes.erase(ntt->m_bvhNodes.begin() + i);
          break;
        }
        i++;
      }
    }
    m_lights.clear();
  }
} // namespace ToolKit
