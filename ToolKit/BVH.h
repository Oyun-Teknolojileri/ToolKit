#pragma once
#pragma once

#include "GeometryTypes.h"
#include "Scene.h"

#include <deque>

namespace ToolKit
{
  class Entity;

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
    EntityPtrArray m_lights;

    bool m_waitingForDeletion = false; //<! Internal variable. True if this node is going to be deleted by conjunction.

    bool m_insideFrustum      = false;

    /**
     * It is guaranteed that if left node is there, there will be right node too
     */
    inline bool Leaf() const { return m_left == nullptr; }
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

   private:
    BVHTree() = delete;

    void UpdateLeaf(BVHNode* node);

   public:
    BVHNode* m_root = nullptr;
    std::vector<BVHNode*> m_nodesToDelete;

    // Utility queue that used to iterate nodes
    std::deque<BVHNode*> m_nextNodes;

    int maxDepth = 0;

   private:
    class BVH* m_bvh               = nullptr;
    int m_maxEntityCountPerBVHNode = 10;
    float m_minBBSize              = 2.0f;
  };

  class TK_API BVH
  {
   public:
    BVH(Scene* scene);
    ~BVH();

    bool ReBuild();
    void Clean();

    void AddEntity(EntityPtr& entity);
    void RemoveEntity(EntityPtr& entity);
    void UpdateEntity(EntityPtr& entity);
    void Update();

    // Query functions
    void PickObject(const Ray& ray, Scene::PickData& pickData, const IDArray& ignoreList, float& closestDistance);
    void PickObject(const Frustum& frustum,
                    Scene::PickDataArray& pickedObjects,
                    const IDArray& ignoreList,
                    const EntityPtrArray& extraList,
                    bool pickPartiallyInside);
    void FrustumCull(const Frustum& frustum, EntityPtrArray& out);
    void SetPointLight(int lightIndex, const BoundingSphere& sphere);
    void SetSpotLight(int lightIndex, const Frustum& frustum);

    // Debug functions
    void GetDebugBVHBoxes(EntityPtrArray& boxes);
    void SanityCheck();

   public:
    // TODO make it private
    BVHTree* m_bvhTree = nullptr;

   private:
    BVH()          = delete;
    Scene* m_scene = nullptr;
    EntityPtrArray m_entitiesToAdd;
    EntityPtrArray m_entitiesToRemove;
    EntityPtrArray m_entitiesToUpdate;
  };

} // namespace ToolKit
