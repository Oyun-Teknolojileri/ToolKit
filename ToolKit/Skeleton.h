#pragma once

#include "Resource.h"

#include <vector>

namespace ToolKit
{

  class TK_API StaticBone
  {
   public:
    explicit StaticBone(String name);
    ~StaticBone();

   public:
    String m_name;
    Mat4 m_inverseWorldMatrix;
  };

  /*
   * This class is used to store dynamic bone information
   */
  class TK_API DynamicBoneMap
  {
   public:
    struct DynamicBone
    {
      // Should be same with matching static bone's element index
      uint boneIndx;
      Node* node;
    };

    // Call after skeleton fills m_bones list
    void Init(const Skeleton* skeleton);
    ~DynamicBoneMap();
    std::unordered_map<String, DynamicBone> boneList;
    TexturePtr boneTransformNodeTexture;
    void UpdateGPUTexture();
    // Find all child bones by recursively searching child bones
    // Then call childProcessFunc (should be recursive to traverse all childs)
    void ForEachRootBone(
        std::function<void(const DynamicBone*)> childProcessFunc) const;
    void ForEachRootBone(std::function<void(DynamicBone*)> childProcessFunc);
    void AddDynamicBone(const String& boneName,
                        DynamicBone& bone,
                        DynamicBone* parent);
  };

  class TK_API Skeleton : public Resource
  {
   public:
    TKResourceType(Skeleton)

    Skeleton();
    explicit Skeleton(String file);
    ~Skeleton();

    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;
    void Load() override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    int GetBoneIndex(String bone);
    StaticBone* GetBone(String bone);

   protected:
    void CopyTo(Resource* other) override;

   public:
    std::vector<StaticBone*> m_bones;
    DynamicBoneMap m_Tpose;
    TexturePtr m_bindPoseTexture;
  };

  class TK_API SkeletonManager : public ResourceManager
  {
   public:
    SkeletonManager();
    virtual ~SkeletonManager();
    bool CanStore(ResourceType t) override;
    ResourcePtr CreateLocal(ResourceType type) override;
  };

} // namespace ToolKit
