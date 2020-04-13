#include "stdafx.h"
#include "Skeleton.h"
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
  }

  Bone::~Bone()
  {
    SafeDel(m_node);
  }

  Skeleton::Skeleton()
  {
    m_node = new Node();
  }

  Skeleton::Skeleton(String file)
  {
    m_file = file;
    m_node = new Node();
  }

  Skeleton::~Skeleton()
  {
    UnInit();
  }

  void Skeleton::Init(bool flushClientSideArray)
  {
    /*
    if (m_initiated)
      return;

    for (auto bone : m_bones)
      bone->m_inverseWorldMatrix = glm::inverse(bone->m_node->GetTransform());

    m_initiated = true;
    */
  }

  void Skeleton::UnInit()
  {
    SafeDel(m_node);
    for (auto bone : m_bones)
      SafeDel(bone);
    m_bones.clear();
    m_initiated = false;
  }

  void Skeleton::Load()
  {
    rapidxml::file<> file(m_file.c_str());
    rapidxml::xml_document<> doc;
    doc.parse<0>(file.data());

    rapidxml::xml_node<>* node = doc.first_node("skeleton");
    if (node == nullptr)
      return;

    for (node = node->first_node("bone"); node; node = node->next_sibling())
      Traverse(node, nullptr);

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
    for (int i = 0; i < (int)m_bones.size(); i++)
    {
      if (m_bones[i]->m_name.compare(bone) == 0)
        return i;
    }

    return -1;
  }

  Bone* Skeleton::GetBone(String bone)
  {
    int index = GetBoneIndex(bone);
    if (index == -1)
      return nullptr;
    return m_bones[index];
  }

  void Skeleton::Traverse(void* data, Bone* parent)
  {
    rapidxml::xml_node<>* node = (rapidxml::xml_node<char>*)(data);
    if (node == nullptr)
      return;

    rapidxml::xml_attribute<>* attr = node->first_attribute("name");

    if (attr == nullptr)
      return;

    Bone* bone = new Bone(attr->value());
    rapidxml::xml_node<>* subNode = node->first_node("translation");
    ExtractXYZFromNode(subNode, bone->m_node->m_translation);

    subNode = node->first_node("scale");
    ExtractXYZFromNode(subNode, bone->m_node->m_scale);

    subNode = node->first_node("rotation");
    ExtractQuatFromNode(subNode, bone->m_node->m_orientation);

    rapidxml::xml_node<>* bindPoseNode = node->first_node("bindPose");
    if (bindPoseNode != nullptr)
    {
      Vec3 ts, scl;
      glm::quat rt;

      rapidxml::xml_node<>* subNode = bindPoseNode->first_node("translation");
      ExtractXYZFromNode(subNode, ts);

      subNode = bindPoseNode->first_node("scale");
      ExtractXYZFromNode(subNode, scl);

      subNode = bindPoseNode->first_node("rotation");
      ExtractQuatFromNode(subNode, rt);

      glm::mat4 tsm;
      tsm = glm::translate(tsm, ts);
      glm::mat4 sclm;
      sclm = glm::scale(sclm, scl);
      glm::mat4 rtm = glm::toMat4(rt);

      bone->m_inverseWorldMatrix = tsm * rtm * sclm;
    }

    AddBone(bone, parent);

    for (subNode = node->first_node("bone"); subNode; subNode = subNode->next_sibling())
      Traverse(subNode, bone);
  }

}
