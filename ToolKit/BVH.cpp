/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "BVH.h"

#include "MathUtil.h"
#include "Renderer.h"
#include "Scene.h"

namespace ToolKit
{
  inline bool IsBVHEntity(const EntityPtr& ntt) { return !ntt->IsA<SkyBase>() && ntt->IsDrawable(); }

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

  void BVH::ReBuild()
  {
    // Get new parameters
    SetParameters(GetEngineSettings().PostProcessing);

    // Clean bvh tree
    Clean();

    // Clean waiting queue
    for (int i = 0; i < m_entitiesToAdd.size(); ++i)
    {
      m_entitiesToAdd[i]->m_isInBVHProcess = false;
    }
    m_entitiesToAdd.clear();
    for (int i = 0; i < m_entitiesToUpdate.size(); ++i)
    {
      m_entitiesToUpdate[i]->m_isInBVHProcess = false;
    }
    m_entitiesToUpdate.clear();
    for (int i = 0; i < m_entitiesToRemove.size(); ++i)
    {
      m_entitiesToRemove[i]->m_isInBVHProcess = false;
    }
    m_entitiesToRemove.clear();

    // Rebuild
    const EntityPtrArray& entities = m_scene->GetEntities();
    m_bvhTree->m_root->m_entites.reserve(entities.size());
    m_bvhTree->m_root->m_aabb = BoundingBox();
    for (int i = 0; i < entities.size(); ++i)
    {
      EntityPtr ntt = entities[i];
      if (IsBVHEntity(ntt))
      {
        m_bvhTree->m_root->m_aabb.UpdateBoundary(ntt->GetBoundingBox(true));
        if (ntt->IsA<Light>())
        {
          m_bvhTree->m_root->m_lights.push_back(ntt);
          ntt->m_bvhNodes.push_back(m_bvhTree->m_root);
        }
        else
        {
          m_bvhTree->m_root->m_entites.push_back(ntt);
          ntt->m_bvhNodes.push_back(m_bvhTree->m_root);
        }
      }
    }
    m_bvhTree->UpdateLeaf(m_bvhTree->m_root, false);
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
    if (!IsBVHEntity(entity))
    {
      return;
    }

    m_addLock.Lock();
    if (!entity->m_isInBVHProcess)
    {
      entity->m_isInBVHProcess = true;
      m_entitiesToAdd.push_back(entity);
    }
    m_addLock.Release();
  }

  void BVH::RemoveEntity(const EntityPtr& entity)
  {
    if (!IsBVHEntity(entity))
    {
      return;
    }

    m_removeLock.Lock();
    if (!entity->m_isInBVHProcess)
    {
      m_entitiesToRemove.push_back(entity);
    }
    m_removeLock.Release();
  }

  void BVH::UpdateEntity(const EntityPtr& entity)
  {
    if (!IsBVHEntity(entity))
    {
      return;
    }

    m_updateLock.Lock();
    if (!entity->m_isInBVHProcess)
    {
      m_entitiesToUpdate.push_back(entity);
    }
    m_updateLock.Release();
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
        ReBuild();
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

    auto entityTestFn = [&](EntityPtr ntt) -> void
    {
      if (contains(ignoreList, ntt->GetIdVal()))
      {
        return;
      }

      const BoundingBox box = ntt->GetBoundingBox(true);
      Scene::PickData pd;
      pd.pickPos = (box.max + box.min) * 0.5f;
      pd.entity  = ntt;
      pickedObjects.push_back(pd);
    };

    while (!m_bvhTree->m_nextNodes.empty())
    {
      BVHNode* currentNode = m_bvhTree->m_nextNodes.front();
      m_bvhTree->m_nextNodes.pop_front();

      // If the nodes parent already is inside of the frustum, no need to check intersection
      if (currentNode->m_frustumTestResult == IntersectResult::Inside)
      {
        if (currentNode->Leaf())
        {
          for (EntityPtr ntt : currentNode->m_entites)
          {
            entityTestFn(ntt);
          }
        }
        else
        {
          currentNode->m_left->m_frustumTestResult  = IntersectResult::Inside;
          currentNode->m_right->m_frustumTestResult = IntersectResult::Inside;

          m_bvhTree->m_nextNodes.push_back(currentNode->m_left);
          m_bvhTree->m_nextNodes.push_back(currentNode->m_right);
        }

        currentNode->m_frustumTestResult = IntersectResult::Outside; // Reset for reuse later ?
        continue;
      }

      IntersectResult res = FrustumBoxIntersection(frustum, currentNode->m_aabb);
      if (res == IntersectResult::Inside)
      {
        // This node and all children nodes of this node do not need to check intersection again
        currentNode->m_frustumTestResult = IntersectResult::Inside;
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

  void BVH::FrustumTest(const Frustum& frustum, EntityRawPtrArray& entities)
  {
    m_bvhTree->m_nextNodes.clear();
    m_bvhTree->m_nextNodes.push_front(m_bvhTree->m_root);
    while (!m_bvhTree->m_nextNodes.empty())
    {
      BVHNode* currentNode = m_bvhTree->m_nextNodes.front();
      m_bvhTree->m_nextNodes.pop_front();

      bool insideFrustum = currentNode->m_insideFrustum;
      IntersectResult res;
      if (!insideFrustum)
      {
        res           = FrustumBoxIntersection(frustum, currentNode->m_aabb);
        insideFrustum = res == IntersectResult::Inside;
      }

      // If the nodes parent already is inside of the frustum, no need to check intersection
      if (insideFrustum)
      {
        if (currentNode->Leaf())
        {
          for (EntityPtr& ntt : currentNode->m_entites)
          {
            entities.push_back(ntt.get());
          }
        }
        else
        {
          currentNode->m_left->m_insideFrustum  = true;
          currentNode->m_right->m_insideFrustum = true;
          m_bvhTree->m_nextNodes.push_back(currentNode->m_left);
          m_bvhTree->m_nextNodes.push_back(currentNode->m_right);
        }
      }
      else if (res == IntersectResult::Intersect)
      {
        if (currentNode->Leaf())
        {
          for (EntityPtr& ntt : currentNode->m_entites)
          {
            const BoundingBox& box = ntt->GetBoundingBox(true);
            IntersectResult res    = FrustumBoxIntersection(frustum, box);

            if (res != IntersectResult::Outside)
            {
              entities.push_back(ntt.get());
            }
          }
        }
        else
        {
          currentNode->m_left->m_insideFrustum  = false;
          currentNode->m_right->m_insideFrustum = false;
          m_bvhTree->m_nextNodes.push_front(currentNode->m_left);
          m_bvhTree->m_nextNodes.push_front(currentNode->m_right);
        }
      }

      currentNode->m_insideFrustum = false;
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
        boxes.push_back(
            Cast<Entity>(CreateBoundingBoxDebugObject(bvhNode->m_aabb,
                                                      Vec3(1.0f, 0.0f, 0.0f) * (bvhNode->m_lights.size() / 16),
                                                      0.75f)));
      }
    }
  }

  void BVH::SanityCheck()
  {
    for (EntityPtr ntt : m_scene->GetEntities())
    {
      assert(ntt->m_bvhNodes.size() <= 1 || ntt->IsA<Light>());
      for (BVHNode* node : ntt->m_bvhNodes)
      {
        assert(node->Leaf() && "Entities should not hold non-leaf bvh nodes");
      }
    }
  }

  void BVH::DistributionQuality(int& totalNtties, int& assignedNtties, float& assignmentPerNtt)
  {
    m_bvhTree->m_nextNodes.clear();
    m_bvhTree->m_nextNodes.push_front(m_bvhTree->m_root);

    assignedNtties = 0;
    while (!m_bvhTree->m_nextNodes.empty())
    {
      BVHNode* currentNode = m_bvhTree->m_nextNodes.front();
      m_bvhTree->m_nextNodes.pop_front();
      if (currentNode != nullptr)
      {
        if (currentNode->Leaf())
        {
          assignedNtties += (int) currentNode->m_entites.size();
        }

        m_bvhTree->m_nextNodes.push_front(currentNode->m_left);
        m_bvhTree->m_nextNodes.push_front(currentNode->m_right);
      }
    }

    totalNtties      = glm::max(1, (int) m_scene->AccessEntityArray().size());
    assignmentPerNtt = (float) assignedNtties / (float) totalNtties;
  }

  const BoundingBox& BVH::GetBVHBoundary() { return m_bvhTree->m_root->m_aabb; }

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
    entity->m_isInBVHProcess = false;

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
      }
      else if (PointLight* point = entity->As<PointLight>())
      {
        entityType = 2;

        point->UpdateShadowCamera();
        lightSphere = point->m_boundingSphereCache;
      }
    }
    else
    {
      entityAABB = entity->GetBoundingBox(true);
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

      node->m_aabb.UpdateBoundary(entity->GetBoundingBox(true));

      if (!node->Leaf())
      {
        // intermediate node

        bool leftTest = false, rightTest = false;
        float distToLeftSquared = FLT_MAX, distToRightSquared = FLT_MAX;

        if (entityType == 1) // spot light
        {
          if (BoxBoxIntersection(lightBBox, node->m_left->m_aabb))
          {
            leftTest          = true;
            distToLeftSquared = glm::distance2(entity->m_node->GetTranslation(), node->m_left->m_aabb.GetCenter());
          }
          if (BoxBoxIntersection(lightBBox, node->m_right->m_aabb))
          {
            rightTest          = true;
            distToRightSquared = glm::distance2(entity->m_node->GetTranslation(), node->m_right->m_aabb.GetCenter());
          }
        }
        else if (entityType == 2) // point light
        {
          if (SphereBoxIntersection(lightSphere, node->m_left->m_aabb))
          {
            leftTest          = true;
            distToLeftSquared = glm::distance2(entity->m_node->GetTranslation(), node->m_left->m_aabb.GetCenter());
          }
          if (SphereBoxIntersection(lightSphere, node->m_right->m_aabb))
          {
            rightTest          = true;
            distToRightSquared = glm::distance2(entity->m_node->GetTranslation(), node->m_right->m_aabb.GetCenter());
          }
        }
        else // entity
        {
          if (BoxBoxIntersection(entityAABB, node->m_left->m_aabb))
          {
            leftTest          = true;
            distToLeftSquared = glm::distance2(entity->m_node->GetTranslation(), node->m_left->m_aabb.GetCenter());
          }
          if (BoxBoxIntersection(entityAABB, node->m_right->m_aabb))
          {
            rightTest          = true;
            distToRightSquared = glm::distance2(entity->m_node->GetTranslation(), node->m_right->m_aabb.GetCenter());
          }
        }

        if (leftTest && rightTest)
        {
          // Lights can be inside multiple BVH nodes, but entities can not
          if (entityType == 0)
          {
            if (distToLeftSquared > distToRightSquared)
            {
              nextNodes.push(node->m_right);
            }
            else
            {
              nextNodes.push(node->m_left);
            }
          }
          else
          {
            nextNodes.push(node->m_right);
            nextNodes.push(node->m_left);
          }
        }
        else if (leftTest)
        {
          nextNodes.push(node->m_left);
        }
        else if (rightTest)
        {
          nextNodes.push(node->m_right);
        }
        else
        {
          // If entity can not go inside any child, put the entity to the nearest node
          const float distLeft  = glm::distance2(entity->m_node->GetTranslation(), node->m_left->m_aabb.GetCenter());
          const float distRight = glm::distance2(entity->m_node->GetTranslation(), node->m_right->m_aabb.GetCenter());

          if (distLeft > distRight)
          {
            node->m_right->m_aabb.UpdateBoundary(entity->GetBoundingBox(true));
            nextNodes.push(node->m_right);
          }
          else
          {
            node->m_left->m_aabb.UpdateBoundary(entity->GetBoundingBox(true));
            nextNodes.push(node->m_left);
          }
        }
      }
      else
      {
        if (entityType == 0) // entity
        {
          node->m_entites.push_back(entity);
        }
        else // light
        {
          node->m_lights.push_back(entity);
        }
        entity->m_bvhNodes.push_back(node);

        UpdateLeaf(node, false);
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
      UpdateLeaf(bvhNode, true);
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

  void BVHTree::UpdateLeaf(BVHNode* node, bool removedFromThisNode)
  {
    if (!node->Leaf())
    {
      assert(false && "Calling UpdateLeaf() on a non-leaf bvh node. Needs to be fixed!");
      return;
    }

    if (removedFromThisNode)
    {
      // Update bounding box
      node->m_aabb = BoundingBox();
      for (EntityPtr& ntt : node->m_entites)
      {
        node->m_aabb.UpdateBoundary(ntt->GetBoundingBox(true));
      }
      // Remove the lights that are not intersecting anymore
      EntityPtrArray lights = node->m_lights; // copy
      node->m_lights.clear();
      for (EntityPtr& lightEntity : lights)
      {
        LightPtr light = Cast<Light>(lightEntity);
        light->UpdateShadowCamera();

        if (light->GetLightType() == Light::LightType::Spot)
        {
          SpotLightPtr spot = Cast<SpotLight>(light);
          if (FrustumBoxIntersection(spot->m_frustumCache, node->m_aabb) != IntersectResult::Outside)
          {
            node->m_lights.push_back(lightEntity);
          }
        }
        else // if (light->GetLightType() == Light::LightType::Point)
        {
          PointLightLightPtr point = Cast<PointLight>(light);
          if (SphereBoxIntersection(point->m_boundingSphereCache, node->m_aabb))
          {
            node->m_lights.push_back(lightEntity);
          }
        }
      }
    }

    // Split or conjunct the BVH node

    const size_t entityCount = node->m_entites.size();
    if (entityCount > m_maxEntityCountPerBVHNode && node->depth < m_maxDepth)
    {
      // split this node if entity number is big

      BVHNode* left           = new BVHNode();
      BVHNode* right          = new BVHNode();
      left->depth             = node->depth + 1;
      right->depth            = node->depth + 1;

      const AxisLabel maxAxis = GetLongestAxis(node->m_aabb);

      const int maxAxisInt    = (int) maxAxis;
      auto sortBasedOnPosFn   = [maxAxisInt](const EntityPtr& e1, const EntityPtr& e2)
      { return e1->m_node->GetTranslation()[maxAxisInt] < e2->m_node->GetTranslation()[maxAxisInt]; };
      std::nth_element(node->m_entites.begin(),
                       node->m_entites.begin() + (node->m_entites.size() / 2),
                       node->m_entites.end(),
                       sortBasedOnPosFn);

      for (auto it = node->m_entites.begin(); it != node->m_entites.begin() + (node->m_entites.size() / 2); ++it)
      {
        left->m_aabb.UpdateBoundary((*it)->GetBoundingBox(true));
        (*it)->m_bvhNodes.push_back(left);
        left->m_entites.push_back(*it);
      }
      for (auto it = node->m_entites.begin() + (node->m_entites.size() / 2); it != node->m_entites.end(); ++it)
      {
        right->m_aabb.UpdateBoundary((*it)->GetBoundingBox(true));
        (*it)->m_bvhNodes.push_back(right);
        right->m_entites.push_back(*it);
      }

      if (left->m_aabb.GetWidth() < m_minBBSize || left->m_aabb.GetHeight() < m_minBBSize ||
          left->m_aabb.GetDepth() < m_minBBSize || right->m_aabb.GetWidth() < m_minBBSize ||
          right->m_aabb.GetHeight() < m_minBBSize || right->m_aabb.GetDepth() < m_minBBSize)
      {
        SafeDel(left);
        SafeDel(right);
      }
      else
      {
        for (EntityPtr& lightEntity : node->m_lights)
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
        for (EntityPtr& ntt : node->m_entites)
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
        for (EntityPtr& ntt : node->m_lights)
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

        UpdateLeaf(left, false);
        if (!right->m_waitingForDeletion)
        {
          UpdateLeaf(right, false);
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

        UpdateLeaf(parentNode, false);
      }
    }
  }

  BVHNode::BVHNode() {}

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
