#include "Skeleton.h"

#include <memory>

#include "Node.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "Util.h"
#include "DebugNew.h"

namespace ToolKit
{

  Bone::Bone(String name)
  {
    m_name = name;
    m_node = new Node();
    m_node->m_inheritScale = true;
  }

  Bone::~Bone()
  {
    // Override orphaning.
    m_node->m_parent = nullptr;
    SafeDel(m_node);
  }

  Skeleton::Skeleton()
  {
    m_node = new Node();
  }

  Skeleton::Skeleton(String file)
  {
    SetFile(file);
    m_node = new Node();
  }

  Skeleton::~Skeleton()
  {
    UnInit();
  }

  void Skeleton::Init(bool flushClientSideArray)
  {
  }

  void Skeleton::UnInit()
  {
    SafeDel(m_node);
    for (Bone* bone : m_bones)
    {
      SafeDel(bone);
    }

    m_bones.clear();
    m_initiated = false;
  }

  void Skeleton::Load()
  {
    XmlFile file(GetFile().c_str());
    XmlDocument doc;
    doc.parse<0>(file.data());

    XmlNode* node = doc.first_node("skeleton");
    if (node == nullptr)
    {
      return;
    }

    for (node = node->first_node("bone"); node; node = node->next_sibling())
    {
      Traverse(node, nullptr);
    }

    m_loaded = true;
  }

  void Skeleton::AddBone(Bone* bone, Bone* parent)
  {
    if (parent == nullptr)
    {
      m_node->AddChild(bone->m_node);
    }
    else
    {
      parent->m_node->AddChild(bone->m_node);
    }

    m_bones.push_back(bone);
  }

  int Skeleton::GetBoneIndex(String bone)
  {
    for (size_t i = 0; i < m_bones.size(); i++)
    {
      if (m_bones[i]->m_name.compare(bone) == 0)
      {
        return static_cast<int> (i);
      }
    }

    return -1;
  }

  Bone* Skeleton::GetBone(String bone)
  {
    int index = GetBoneIndex(bone);
    if (index == -1)
    {
      return nullptr;
    }
    return m_bones[index];
  }

  void Skeleton::Traverse(XmlNode* node, Bone* parent)
  {
    if (node == nullptr)
    {
      return;
    }

    XmlAttribute* attr = node->first_attribute("name");

    if (attr == nullptr)
    {
      return;
    }

    Bone* bone = new Bone(attr->value());
    XmlNode* subNode = node->first_node("translation");
    ReadVec(subNode, bone->m_node->m_translation);

    subNode = node->first_node("scale");
    ReadVec(subNode, bone->m_node->m_scale);

    subNode = node->first_node("rotation");
    ReadVec(subNode, bone->m_node->m_orientation);

    XmlNode* bindPoseNode = node->first_node("bindPose");
    if (bindPoseNode != nullptr)
    {
      Vec3 ts, scl;
      Quaternion rt;

      XmlNode* subNode = bindPoseNode->first_node("translation");
      ReadVec(subNode, ts);

      subNode = bindPoseNode->first_node("scale");
      ReadVec(subNode, scl);

      subNode = bindPoseNode->first_node("rotation");
      ReadVec(subNode, rt);

      Mat4 tsm;
      tsm = glm::translate(tsm, ts);
      Mat4 sclm;
      sclm = glm::scale(sclm, scl);
      Mat4 rtm = glm::toMat4(rt);

      bone->m_inverseWorldMatrix = tsm * rtm * sclm;
    }

    AddBone(bone, parent);

    for
    (
      subNode = node->first_node("bone");
      subNode;
      subNode = subNode->next_sibling()
    )
    {
      Traverse(subNode, bone);
    }
  }

  SkeletonManager::SkeletonManager()
  {
    m_type = ResourceType::Skeleton;
  }

  SkeletonManager::~SkeletonManager()
  {
  }

  bool SkeletonManager::CanStore(ResourceType t)
  {
    if (t == ResourceType::Skeleton)
    {
      return true;
    }

    return false;
  }

  ResourcePtr SkeletonManager::CreateLocal(ResourceType type)
  {
    ResourcePtr res;
    if (type == ResourceType::Skeleton)
    {
      res = std::make_shared<Skeleton> ();
    }

    assert(res != nullptr);
    return res;
  }

}  // namespace ToolKit
