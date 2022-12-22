#include "Skeleton.h"

#include "GL/glew.h"
#include "Node.h"
#include "ToolKit.h"
#include "Util.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

#include <memory>

#include "DebugNew.h"

namespace ToolKit
{

  StaticBone::StaticBone(String name) { m_name = name; }

  StaticBone::~StaticBone()
  {
    /*
    m_node->m_parent = nullptr;
    m_node->m_children.clear();*/
  }

  // Create a texture such that there is mat4x4 per bone
  TexturePtr CreateBoneTransformTexture(const Skeleton* skeleton)
  {
    TexturePtr ptr     = std::make_shared<Texture>();
    ptr->m_floatFormat = true;
    ptr->m_height      = 1;
    ptr->m_width       = static_cast<int>(skeleton->m_bones.size()) * 4;
    ptr->m_name        = skeleton->m_name + " BindPoseTexture";

    glGenTextures(1, &ptr->m_textureId);
    glBindTexture(GL_TEXTURE_2D, ptr->m_textureId);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA32F,
                 ptr->m_width,
                 ptr->m_height,
                 0,
                 GL_RGBA,
                 GL_FLOAT,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    ptr->m_initiated = true;
    ptr->m_loaded    = true;
    return ptr;
  }

  void uploadBoneMatrix(Mat4 mat, TexturePtr& ptr, uint boneIndx)
  {
    glBindTexture(GL_TEXTURE_2D, ptr->m_textureId);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    boneIndx * 4,
                    0,
                    4,
                    1,
                    GL_RGBA,
                    GL_FLOAT,
                    &mat);
  };

  void DynamicBoneMap::AddDynamicBone(const String& boneName,
                                      DynamicBoneMap::DynamicBone& bone,
                                      DynamicBoneMap::DynamicBone* parent)
  {
    if (parent)
    {
      parent->node->AddChild(bone.node);
    }

    boneList.insert(std::make_pair(boneName, bone));
  }

  void DynamicBoneMap::Init(const Skeleton* skeleton)
  {
    assert(skeleton->m_bones.size() != 0);

    // Create a copy of each bone (but create new pointers for Nodes)
    for (const auto& tPoseBone : skeleton->m_Tpose.boneList)
    {
      DynamicBone newBone;
      newBone.boneIndx = tPoseBone.second.boneIndx;
      newBone.node     = new Node();
      *newBone.node    = *tPoseBone.second.node;
      boneList.insert(std::make_pair(tPoseBone.first, newBone));
    }
    // Fix parent/child relations
    //  ChildBone's node points to Skeleton's list rather than newly created one
    for (const auto& tPoseBone : skeleton->m_Tpose.boneList)
    {
      auto& newBoneIter = boneList[tPoseBone.first];

      // Find the index of the each child bone in T-Pose list
      // Then set newBone's child bone pointer to the child
      for (Node*& childBoneNode : newBoneIter.node->m_children)
      {
        for (const auto& tPoseSearchBone : skeleton->m_Tpose.boneList)
        {
          if (tPoseSearchBone.second.node == childBoneNode)
          {
            childBoneNode           = boneList[tPoseSearchBone.first].node;
            childBoneNode->m_parent = newBoneIter.node;
            break;
          }
        }
      }
    }

    boneTransformNodeTexture = CreateBoneTransformTexture(skeleton);
  }

  void DynamicBoneMap::UpdateGPUTexture()
  {
    for (auto& dBoneIter : boneList)
    {
      const String& name = dBoneIter.first;
      DynamicBone& dBone = dBoneIter.second;
      uploadBoneMatrix(dBone.node->GetTransform(TransformationSpace::TS_WORLD),
                       boneTransformNodeTexture,
                       dBone.boneIndx);
    }
  }

  DynamicBoneMap::~DynamicBoneMap()
  {
    for (auto& dBoneIter : boneList)
    {
      dBoneIter.second.node->m_parent = nullptr;
      dBoneIter.second.node->m_children.clear();
      SafeDel(dBoneIter.second.node);
      dBoneIter.second.node = nullptr;
    }
    boneList.clear();
  }

  Skeleton::Skeleton() {}

  Skeleton::Skeleton(String file) { SetFile(file); }

  Skeleton::~Skeleton() { UnInit(); }

  void Skeleton::Init(bool flushClientSideArray) {}

  // Find skeleton's each child bone from its child nodes
  // Then call childProcessFunc (should be recursive to traverse all childs)
  void DynamicBoneMap::ForEachRootBone(
      std::function<void(const DynamicBone*)> childProcessFunc) const
  {
    // For each parent bone of the skeleton, access child bones recursively
    for (auto& bone : boneList)
    {
      if (bone.second.node->m_parent == nullptr)
      {
        Node* childNode              = bone.second.node;
        // Dynamic child node
        const DynamicBone* childBone = nullptr;
        for (auto& boneSearch : boneList)
        {
          if (boneSearch.second.node == childNode)
          {
            childBone = &boneSearch.second;
            break;
          }
        }
        // If there is a child bone
        if (childBone)
        {
          childProcessFunc(childBone);
        }
      }
    }
  }

  void DynamicBoneMap::ForEachRootBone(
      std::function<void(DynamicBone*)> childProcessFunc)
  {
    // For each parent bone of the skeleton, access child bones recursively
    for (auto& bone : boneList)
    {
      if (bone.second.node->m_parent == nullptr)
      {
        Node* childNode        = bone.second.node;
        // Dynamic child node
        DynamicBone* childBone = nullptr;
        for (auto& boneSearch : boneList)
        {
          if (boneSearch.second.node == childNode)
          {
            childBone = &boneSearch.second;
            break;
          }
        }
        // If there is a child bone
        if (childBone)
        {
          childProcessFunc(childBone);
        }
      }
    }
  }

  void Skeleton::UnInit()
  {
    /*
    uint32_t deletedCount = 0;
    std::function<void(const StaticBone*)> deleteBone =
        [this, &deleteBone, &deletedCount](
            const StaticBone* parentBone) -> void {
      for (Node* childNode : parentBone->m_node->m_children)
      {
        // Find child bone
        StaticBone* childBone = nullptr;
        for (StaticBone* boneSearch : this->m_bones)
        {
          if (boneSearch->m_node == childNode)
          {
            childBone = boneSearch;
            break;
          }
        }
        // If child node is a bone of the skeleton
        if (childBone)
        {
          deleteBone(childBone);
        }
      }
      deletedCount++;
      SafeDel(parentBone);
    };
    ForEachChildBoneNode(this, deleteBone);
    m_node->m_children.clear();
    SafeDel(m_node);
    */

    for (StaticBone* sBone : m_bones)
    {
      SafeDel(sBone);
    }
    m_bones.clear();
    m_initiated = false;
  }

  void Skeleton::Load()
  {
    XmlFilePtr file = GetFileManager()->GetXmlFile(GetFile());
    XmlDocument doc;
    doc.parse<0>(file->data());

    if (XmlNode* node = doc.first_node("skeleton"))
    {
      DeSerialize(&doc, node);
      m_loaded = true;
    }
  }

  void WriteBone(const Skeleton* skeleton,
                 const DynamicBoneMap::DynamicBone* dBone,
                 XmlDocument* doc,
                 XmlNode* parentNode)
  {
    const StaticBone* sBone = skeleton->m_bones[dBone->boneIndx];
    XmlNode* boneXmlNode    = CreateXmlNode(doc, "bone", parentNode);

    WriteAttr(boneXmlNode, doc, "name", sBone->m_name.c_str());

    auto writeTransformFnc =
        [doc](XmlNode* parent, Vec3 tra, Quaternion rot, Vec3 scale)
    {
      XmlNode* traNode = CreateXmlNode(doc, "translation", parent);
      WriteVec(traNode, doc, tra);

      XmlNode* rotNode = CreateXmlNode(doc, "rotation", parent);
      WriteVec(rotNode, doc, rot);

      XmlNode* scaleNode = CreateXmlNode(doc, "scale", parent);
      WriteVec(scaleNode, doc, scale);
    };

    // Bone Node Transform
    writeTransformFnc(boneXmlNode,
                      dBone->node->GetTranslation(),
                      dBone->node->GetOrientation(),
                      dBone->node->GetScale());

    // Bind Pose Transform
    XmlNode* bindPoseNode = CreateXmlNode(doc, "bindPose", boneXmlNode);
    {
      Vec3 tra, scale;
      Quaternion rot;
      DecomposeMatrix(sBone->m_inverseWorldMatrix, &tra, &rot, &scale);
      writeTransformFnc(bindPoseNode, tra, rot, scale);
    }

    for (Node* childNode : dBone->node->m_children)
    {
      // Find child bone
      const DynamicBoneMap::DynamicBone* childBone = nullptr;
      for (auto& boneSearch : skeleton->m_Tpose.boneList)
      {
        if (boneSearch.second.node == childNode)
        {
          childBone = &boneSearch.second;
          break;
        }
      }
      // If child node is a bone of the skeleton
      if (childBone)
      {
        WriteBone(skeleton, childBone, doc, boneXmlNode);
      }
    }
  }

  void Skeleton::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* container = CreateXmlNode(doc, "skeleton", parent);

    auto writeBoneFnc =
        [doc, container, this](
            const DynamicBoneMap::DynamicBone* childBone) -> void
    { WriteBone(this, childBone, doc, container); };
    m_Tpose.ForEachRootBone(writeBoneFnc);
  }

  void Traverse(XmlNode* node,
                DynamicBoneMap::DynamicBone* parent,
                Skeleton* skeleton)
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

    StaticBone* sBone                 = new StaticBone(attr->value());
    DynamicBoneMap::DynamicBone dBone = {};
    dBone.node                        = new Node();
    dBone.node->m_inheritScale        = true;
    XmlNode* subNode                  = node->first_node("translation");
    Vec3 temp;
    ReadVec(subNode, temp);
    dBone.node->SetTranslation(temp);

    subNode = node->first_node("scale");
    ReadVec(subNode, temp);
    dBone.node->SetScale(temp);

    Quaternion tempRot;
    subNode = node->first_node("rotation");
    ReadVec(subNode, tempRot);
    dBone.node->SetOrientation(tempRot);

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
      sclm                        = glm::scale(sclm, scl);
      Mat4 rtm                    = glm::toMat4(rt);

      sBone->m_inverseWorldMatrix = tsm * rtm * sclm;
    }

    dBone.boneIndx = uint(skeleton->m_bones.size());
    skeleton->m_bones.push_back(sBone);
    skeleton->m_Tpose.AddDynamicBone(sBone->m_name, dBone, parent);

    for (XmlNode* subNode = node->first_node("bone"); subNode;
         subNode          = subNode->next_sibling())
    {
      Traverse(subNode, &dBone, skeleton);
    }
  }

  void Skeleton::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    for (XmlNode* node = parent->first_node("bone"); node;
         node          = node->next_sibling())
    {
      Traverse(node, nullptr, this);
    }

    m_bindPoseTexture = CreateBoneTransformTexture(this);
    for (uint64_t boneIndx = 0; boneIndx < m_bones.size(); boneIndx++)
    {
      uploadBoneMatrix(m_bones[boneIndx]->m_inverseWorldMatrix,
                       m_bindPoseTexture,
                       static_cast<uint>(boneIndx));
    }

    m_loaded = true;
  }

  int Skeleton::GetBoneIndex(String bone)
  {
    for (size_t i = 0; i < m_bones.size(); i++)
    {
      if (m_bones[i]->m_name.compare(bone) == 0)
      {
        return static_cast<int>(i);
      }
    }

    return -1;
  }

  StaticBone* Skeleton::GetBone(String bone)
  {
    int index = GetBoneIndex(bone);
    if (index == -1)
    {
      return nullptr;
    }
    return m_bones[index];
  }

  void Skeleton::CopyTo(Resource* other)
  {
    Skeleton* dst = static_cast<Skeleton*>(other);
    dst->SetFile(GetFile());
    dst->Load();
  }

  SkeletonManager::SkeletonManager() { m_type = ResourceType::Skeleton; }

  SkeletonManager::~SkeletonManager() {}

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
      res = std::make_shared<Skeleton>();
    }

    assert(res != nullptr);
    return res;
  }

} // namespace ToolKit
