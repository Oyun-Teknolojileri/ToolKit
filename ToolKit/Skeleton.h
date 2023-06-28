/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "Resource.h"

#include <vector>

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
    std::unordered_map<String, DynamicBone> boneList;
    TexturePtr boneTransformNodeTexture;
    void UpdateGPUTexture();
    // Find all child bones by recursively searching child bones
    // Then call childProcessFunc (should be recursive to traverse all childs)
    void ForEachRootBone(std::function<void(const DynamicBone*)> childProcessFunc) const;
    void ForEachRootBone(std::function<void(DynamicBone*)> childProcessFunc);
    void AddDynamicBone(const String& boneName, DynamicBone& bone, DynamicBone* parent);
  };

  class TK_API Skeleton : public Resource
  {
   public:
    TKResourceType(Skeleton)

    Skeleton();
    explicit Skeleton(const String& file);
    ~Skeleton();

    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;
    void Load() override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerializeImp(XmlDocument* doc, XmlNode* parent) override;

    int GetBoneIndex(const String& bone);
    StaticBone* GetBone(const String& bone);

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
