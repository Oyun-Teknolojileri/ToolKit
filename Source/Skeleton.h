#pragma once

#include "Resource.h"


namespace ToolKit
{
  class Node;

  class Bone
  {
  public:
    Bone(std::string name);
    ~Bone();

  public:
    std::string m_name;
    Node* m_node;
    glm::mat4 m_inverseWorldMatrix;
  };

  class Skeleton : public Resource
  {
  public:
    Skeleton();
    Skeleton(std::string file);
    ~Skeleton();

    void Init(bool flushClientSideArray = true);
    void Load();
    void AddBone(Bone* bone, Bone* parent = nullptr);
    int GetBoneIndex(std::string bone);
    Bone* GetBone(std::string bone);

  private:
    void Traverse(void* data, Bone* parent);

  public:
    Node* m_node;
    std::vector<Bone*> m_bones;
  };

}