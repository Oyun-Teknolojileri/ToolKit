/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Skeleton.h"

#include "FileManager.h"
#include "MathUtil.h"
#include "Node.h"
#include "TKOpenGL.h"
#include "TKStats.h"
#include "Texture.h"
#include "ToolKit.h"
#include "Util.h"

#include "DebugNew.h"

namespace ToolKit
{

  StaticBone::StaticBone(const String& name) { m_name = name; }

  StaticBone::~StaticBone() {}

  // Create a texture such that there is mat4x4 per bone
  TexturePtr CreateBoneTransformTexture(const Skeleton* skeleton)
  {
    TexturePtr ptr = MakeNewPtr<Texture>();
    ptr->m_height  = 1;
    ptr->m_width   = (int) (skeleton->m_bones.size()) * 4;
    TextureSettings set;
    set.GenerateMipMap  = false;
    set.InternalFormat  = GraphicTypes::FormatRGBA32F;
    set.MinFilter       = GraphicTypes::SampleNearest;
    set.MipMapMinFilter = GraphicTypes::SampleNearestMipmapNearest;
    set.Type            = GraphicTypes::TypeFloat;
    ptr->SetTextureSettings(set);
    ptr->m_name = skeleton->m_name + " BindPoseTexture";

    glGenTextures(1, &ptr->m_textureId);
    glBindTexture(GL_TEXTURE_2D, ptr->m_textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ptr->m_width, ptr->m_height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (TKStats* tkStats = GetTKStats())
    {
      tkStats->AddVRAMUsageInBytes(ptr->m_width * ptr->m_height * 16);
    }

    ptr->m_initiated = true;
    ptr->m_loaded    = true;
    return ptr;
  }

  void uploadBoneMatrix(Mat4 mat, TexturePtr& ptr, uint boneIndx)
  {
    glBindTexture(GL_TEXTURE_2D, ptr->m_textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, boneIndx * 4, 0, 4, 1, GL_RGBA, GL_FLOAT, &mat);
  };

  // DynamicBoneMap
  //////////////////////////////////////////////////////////////////////////

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
    if (skeleton->m_bones.empty())
    {
      return;
    }

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

  void DynamicBoneMap::ForEachRootBone(std::function<void(DynamicBone*)> childProcessFunc)
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

  // Skeleton
  //////////////////////////////////////////////////////////////////////////

  TKDefineClass(Skeleton, Resource);

  Skeleton::Skeleton() {}

  Skeleton::Skeleton(const String& file) { SetFile(file); }

  Skeleton::~Skeleton() { UnInit(); }

  void Skeleton::Init(bool flushClientSideArray) {}

  // Find skeleton's each child bone from its child nodes
  // Then call childProcessFunc (should be recursive to traverse all childs)
  void DynamicBoneMap::ForEachRootBone(std::function<void(const DynamicBone*)> childProcessFunc) const
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

  void Skeleton::UnInit()
  {
    if (m_initiated)
    {
      for (StaticBone* sBone : m_bones)
      {
        SafeDel(sBone);
      }
      m_bones.clear();

      m_bindPoseTexture = nullptr;

      m_initiated = false;
    }
  }

  void Skeleton::Load()
  {
    if (!m_loaded)
    {
      ParseDocument("skeleton");
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

    auto writeTransformFnc = [doc](XmlNode* parent, Vec3 tra, Quaternion rot, Vec3 scale)
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

  XmlNode* Skeleton::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* container = CreateXmlNode(doc, "skeleton", parent);

    auto writeBoneFnc  = [doc, container, this](const DynamicBoneMap::DynamicBone* childBone) -> void
    { WriteBone(this, childBone, doc, container); };
    m_Tpose.ForEachRootBone(writeBoneFnc);

    return container;
  }

  void Traverse(XmlNode* node, DynamicBoneMap::DynamicBone* parent, Skeleton* skeleton)
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

    for (XmlNode* subNode = node->first_node("bone"); subNode; subNode = subNode->next_sibling())
    {
      Traverse(subNode, &dBone, skeleton);
    }
  }

  XmlNode* Skeleton::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {

    for (XmlNode* node = parent->first_node("bone"); node; node = node->next_sibling())
    {
      Traverse(node, nullptr, this);
    }

    m_bindPoseTexture = CreateBoneTransformTexture(this);
    for (uint64_t boneIndx = 0; boneIndx < m_bones.size(); boneIndx++)
    {
      uploadBoneMatrix(m_bones[boneIndx]->m_inverseWorldMatrix, m_bindPoseTexture, static_cast<uint>(boneIndx));
    }

    m_initiated = true;

    return nullptr;
  }

  int Skeleton::GetBoneIndex(const String& bone)
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

  StaticBone* Skeleton::GetBone(const String& bone)
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

  // SkeletonManager
  //////////////////////////////////////////////////////////////////////////

  SkeletonManager::~SkeletonManager() {}

  SkeletonManager::SkeletonManager() { m_baseType = Skeleton::StaticClass(); }

  bool SkeletonManager::CanStore(ClassMeta* Class)
  {
    if (Class == Skeleton::StaticClass())
    {
      return true;
    }

    return false;
  }

} // namespace ToolKit
