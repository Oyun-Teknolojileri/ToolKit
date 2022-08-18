#include "Skeleton.h"

#include <memory>

#include "Node.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "Util.h"
#include "DebugNew.h"
#include "ToolKit.h"
#include "GL/glew.h"

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
    NodePtrArray& childsOfParent = m_node->m_parent->m_children;
    for (uint childIndx = 0; childIndx < childsOfParent.size(); childIndx)
    {
      if (m_node == childsOfParent[childIndx])
      {
        childsOfParent.erase(childsOfParent.begin() + childIndx);
      }
      else
      {
        childIndx++;
      }
    }
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

  // Find skeleton's each child bone from its child nodes
  // Then call childProcessFunc (should be recursive to traverse all childs)
  void ForEachChildBoneNode
  (
    const Skeleton* skltn,
    std::function<void(const Bone*)> childProcessFunc
  )
  {
    // For each parent bone of the skeleton, write bones recursively
    for (Node* childNode : skltn->m_node->m_children)
    {
      // Find child node
      Bone* childBone = nullptr;
      for (Bone* boneSearch : skltn->m_bones)
      {
        if (boneSearch->m_node = childNode)
        {
          childBone = boneSearch;
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

  void Skeleton::UnInit()
  {
    std::function<void(const Bone*)> deleteBone =
    [this, &deleteBone]
    (
      const Bone* parentBone
    ) -> void
    {
      for (Node* childNode : parentBone->m_node->m_children)
      {
        // Find child bone
        Bone* childBone = nullptr;
        for (Bone* boneSearch : this->m_bones)
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
      SafeDel(parentBone);
    };
    ForEachChildBoneNode(this, deleteBone);
    SafeDel(m_node);

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

  void WriteBone
  (
    const Skeleton* skeleton,
    const Bone* bone,
    XmlDocument* doc,
    XmlNode* parentNode
  )
  {
    XmlNode* boneXmlNode = CreateXmlNode(doc, "bone", parentNode);

    WriteAttr(boneXmlNode, doc, "name", bone->m_name.c_str());

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
    writeTransformFnc
    (
      boneXmlNode,
      bone->m_node->GetTranslation(),
      bone->m_node->GetOrientation(),
      bone->m_node->GetScale()
    );

    // Bind Pose Transform
    XmlNode* bindPoseNode = CreateXmlNode(doc, "bindPose", boneXmlNode);
    {
      Vec3 tra, scale;
      Quaternion rot;
      DecomposeMatrix(bone->m_inverseWorldMatrix, &tra, &rot, &scale);
      writeTransformFnc(bindPoseNode, tra, rot, scale);
    }

    for (Node* childNode : bone->m_node->m_children)
    {
      // Find child bone
      Bone* childBone = nullptr;
      for(Bone* boneSearch : skeleton->m_bones)
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
        WriteBone(skeleton, childBone, doc, boneXmlNode);
      }
    }
  }
  void Skeleton::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* container = CreateXmlNode(doc, "skeleton", parent);

    auto writeBoneFnc =
      [doc, container, this]
    (const Bone* childBone) -> void
    {
      WriteBone
      (
        this,
        childBone,
        doc,
        container
      );
    };
    ForEachChildBoneNode(this, writeBoneFnc);
  }
  void Skeleton::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    for
    (
      XmlNode* node = parent->first_node("bone");
      node;
      node = node->next_sibling()
    )
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
  void Skeleton::UpdateTransformationTexture()
  {
    auto createBoneTexture = [this](TexturePtr& ptr)
    {
      ptr = std::make_shared<Texture>();
      ptr->m_floatFormat = true;
      ptr->m_height = 1;
      ptr->m_width = static_cast<int>(m_bones.size()) * 4;
      ptr->m_name = m_name + " BindPoseTexture";

      glGenTextures(1, &ptr->m_textureId);
      glBindTexture(GL_TEXTURE_2D, ptr->m_textureId);
      glTexImage2D
      (
        GL_TEXTURE_2D,
        0,
        GL_RGBA32F,
        ptr->m_width,
        ptr->m_height,
        0,
        GL_RGBA,
        GL_FLOAT,
        nullptr
      );
      glTexParameteri
      (
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_NEAREST
      );
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      ptr->m_initiated = true;
      ptr->m_loaded = true;
    };
    auto uploadBoneMatrix = [](Mat4 mat, uint boneIndx)
    {
      glTexSubImage2D
      (
        GL_TEXTURE_2D,
        0,
        boneIndx * 4,
        0,
        4,
        1,
        GL_RGBA,
        GL_FLOAT,
        &mat
      );
    };


    if (m_bindPoseTexture == nullptr)
    {
      createBoneTexture(m_bindPoseTexture);
      for (uint64_t boneIndx = 0; boneIndx < m_bones.size(); boneIndx++)
      {
        uploadBoneMatrix
        (
          m_bones[boneIndx]->m_inverseWorldMatrix,
          static_cast<uint>(boneIndx)
        );
      }
    }
    if (m_boneTransformTexture == nullptr)
    {
      createBoneTexture(m_boneTransformTexture);
    }
    for (uint bnIndx = 0; bnIndx < static_cast<uint>(m_bones.size()); bnIndx++)
    {
      uploadBoneMatrix
      (
        m_bones[bnIndx]->m_node->GetTransform(TransformationSpace::TS_WORLD),
        (uint)bnIndx
      );
    }
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
