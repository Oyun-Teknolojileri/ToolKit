/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Resource.h"
#include "ResourceManager.h"
#include <map>

namespace ToolKit
{

  class TK_API StaticBone
  {
   public:
    explicit StaticBone(const String& name);
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
    std::map<String, DynamicBone> boneList;
    TexturePtr boneTransformNodeTexture;
    void UpdateGPUTexture(std::vector<StaticBone*>& staticBones);
    // Find all child bones by recursively searching child bones
    // Then call childProcessFunc (should be recursive to traverse all childs)
    void ForEachRootBone(std::function<void(const DynamicBone*)> childProcessFunc) const;
    void ForEachRootBone(std::function<void(DynamicBone*)> childProcessFunc);
    void AddDynamicBone(const String& boneName, DynamicBone& bone, DynamicBone* parent);
  };

  class TK_API Skeleton : public Resource
  {
   public:
    TKDeclareClass(Skeleton, Resource);

    Skeleton();
    explicit Skeleton(const String& file);
    virtual ~Skeleton();

    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;
    void Load() override;

    int GetBoneIndex(const String& bone);
    StaticBone* GetBone(const String& bone);

   protected:
    void CopyTo(Resource* other) override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

   public:
    std::vector<StaticBone*> m_bones;
    DynamicBoneMap m_Tpose;
    TexturePtr m_bindPoseTexture = nullptr;
  };

  class TK_API SkeletonManager : public ResourceManager
  {
   public:
    SkeletonManager();
    virtual ~SkeletonManager();
    bool CanStore(ClassMeta* Class) override;
  };

} // namespace ToolKit
