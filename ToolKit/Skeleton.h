#pragma once

#include <vector>
#include "Resource.h"

namespace ToolKit
{

  class TK_API Bone
  {
   public:
    explicit Bone(String name);
    ~Bone();

   public:
    String m_name;
    Node* m_node;
    Mat4 m_inverseWorldMatrix;
  };

  class TK_API Skeleton : public Resource
  {
   public:
    TKResouceType(Skeleton)

    Skeleton();
    explicit Skeleton(String file);
    ~Skeleton();

    void Init(bool flushClientSideArray = true) override;
    void UnInit() override;
    void Load() override;

    void AddBone(Bone* bone, Bone* parent = nullptr);
    int GetBoneIndex(String bone);
    Bone* GetBone(String bone);

   private:
    void Traverse(XmlNode* node, Bone* parent);

   public:
    Node* m_node;
    std::vector<Bone*> m_bones;
  };

  class TK_API SkeletonManager : public ResourceManager
  {
   public:
    SkeletonManager();
    virtual ~SkeletonManager();
    bool CanStore(ResourceType t) override;
    ResourcePtr CreateLocal(ResourceType type) override;
  };

}  // namespace ToolKit
