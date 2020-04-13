#pragma once

#include "Resource.h"

namespace ToolKit
{
  class Node;

  class Bone
  {
  public:
    Bone(String name);
    ~Bone();

  public:
    String m_name;
    Node* m_node;
    Mat4 m_inverseWorldMatrix;
  };

  class Skeleton : public Resource
  {
  public:
    Skeleton();
    Skeleton(String file);
    ~Skeleton();

    virtual void Init(bool flushClientSideArray = true) override;
		virtual void UnInit() override;
    virtual void Load() override;

    void AddBone(Bone* bone, Bone* parent = nullptr);
    int GetBoneIndex(String bone);
    Bone* GetBone(String bone);

  private:
    void Traverse(void* data, Bone* parent);

  public:
    Node* m_node;
    std::vector<Bone*> m_bones;
  };

}