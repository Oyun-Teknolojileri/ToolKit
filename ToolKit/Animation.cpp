/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Animation.h"

#include "AnimationControllerComponent.h"
#include "Common/base64.h"
#include "Entity.h"
#include "FileManager.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "Node.h"
#include "Skeleton.h"
#include "ToolKit.h"
#include "Util.h"

#include "DebugNew.h"

static constexpr bool SERIALIZE_ANIMATION_AS_BINARY = false;

namespace ToolKit
{

  TKDefineClass(Animation, Resource);

  Animation::Animation() {}

  Animation::Animation(const String& file) : Animation() { SetFile(file); }

  Animation::~Animation() { UnInit(); }

  void Animation::GetPose(Node* node, float time)
  {
    if (m_keys.empty())
    {
      return;
    }

    float ratio;
    int key1, key2;
    std::vector<Key>& keys = m_keys.begin()->second;
    GetNearestKeys(keys, key1, key2, ratio, time);

    int keySize = static_cast<int>(keys.size());
    if (keys.size() <= key1 || key1 == -1)
    {
      return;
    }

    if (keys.size() <= key2 || key2 == -1)
    {
      return;
    }

    Key k1              = keys[key1];
    Key k2              = keys[key2];

    Vec3 positon        = Interpolate(k1.m_position, k2.m_position, ratio);
    Quaternion rotation = glm::slerp(k1.m_rotation, k2.m_rotation, ratio);
    Vec3 scale          = Interpolate(k1.m_scale, k2.m_scale, ratio);

    node->SetLocalTransforms(positon, rotation, scale);
  }

  void Animation::GetPose(const SkeletonComponentPtr& skeleton, float time, BlendTarget* blendTarget)
  {
    if (m_keys.empty())
    {
      return;
    }

    float ratio;
    int key1, key2;
    Vec3 translation;
    Quaternion orientation;
    Vec3 scale;

    EntityPtr owner = skeleton->OwnerEntity();
    for (auto& dBoneIter : skeleton->m_map->boneList)
    {
      auto entry = m_keys.find(dBoneIter.first);
      if (entry == m_keys.end())
      {
        continue;
      }

      GetNearestKeys(entry->second, key1, key2, ratio, time);

      // Sanity checks
      int keySize = static_cast<int>(entry->second.size());
      if (keySize <= key1 || keySize <= key2)
      {
        continue;
      }

      if (key1 == -1 || key2 == -1)
      {
        continue;
      }

      Key k1                             = entry->second[key1];
      Key k2                             = entry->second[key2];
      DynamicBoneMap::DynamicBone& dBone = dBoneIter.second;

      translation                        = Interpolate(k1.m_position, k2.m_position, ratio);
      orientation                        = glm::slerp(k1.m_rotation, k2.m_rotation, ratio);
      scale                              = Interpolate(k1.m_scale, k2.m_scale, ratio);

      // Blending with next animation
      if (blendTarget != nullptr && blendTarget->Blend)
      {
        // Calculate the current time of the target animation.
        float targetAnimTime = time - m_duration + blendTarget->OverlapTime;
        if (targetAnimTime >= 0.0f) // Start blending.
        {
          // Calculate blend ratio between source - target.
          float blendRatio = targetAnimTime / blendTarget->OverlapTime;
          // Find the corresponding bone's transforms on target anim.
          auto targetEntry = blendTarget->TargetAnim->m_keys.find(dBoneIter.first);
          int targetKey1, targetKey2;
          float targetRatio;
          blendTarget->TargetAnim->GetNearestKeys(targetEntry->second,
                                                  targetKey1,
                                                  targetKey2,
                                                  targetRatio,
                                                  targetAnimTime);
          Key targetK1            = targetEntry->second[targetKey1];
          Key targetK2            = targetEntry->second[targetKey2];

          Vec3 translationT       = Interpolate(targetK1.m_position, targetK2.m_position, targetRatio);
          Quaternion orientationT = glm::slerp(targetK1.m_rotation, targetK2.m_rotation, targetRatio);
          Vec3 scaleT             = Interpolate(targetK1.m_scale, targetK2.m_scale, targetRatio);

          // For the source anims with root motion or rotation,
          // Target anim is offseted from it's root bone.
          if (dBoneIter.first == blendTarget->RootBone)
          {
            Vec3 entityScale       = owner->m_node->GetScale();
            float translationCoeff = (1 / entityScale.x);
            translationT           = translationT + (blendTarget->TranslationOffset * translationCoeff);
            orientationT           = orientationT * blendTarget->OrientationOffset;
          }
          // Blend animations.
          translation = Interpolate(translation, translationT, blendRatio);
          orientation = glm::slerp(orientation, orientationT, blendRatio);
          scale       = Interpolate(scale, scaleT, blendRatio);
        }
      }

      dBone.node->SetLocalTransforms(translation, orientation, scale);
    }
    skeleton->isDirty = true;
  }

  void Animation::GetPose(Node* node, int frame) { GetPose(node, frame * 1.0f / m_fps); }

  void Animation::Load()
  {
    if (!m_loaded)
    {
      ParseDocument("anim");
      m_loaded = true;
    }
  }

  XmlNode* Animation::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* container      = CreateXmlNode(doc, "anim", parent);

    char* fpsValueStr       = doc->allocate_string(std::to_string(m_fps).c_str());
    XmlAttribute* fpsAttrib = doc->allocate_attribute("fps", fpsValueStr);
    container->append_attribute(fpsAttrib);

    char* durationValueStr  = doc->allocate_string(std::to_string(m_duration).c_str());
    XmlAttribute* durAttrib = doc->allocate_attribute("duration", durationValueStr);
    container->append_attribute(durAttrib);

    BoneKeyArrayMap::const_iterator iterator;
    for (const auto& [boneName, keys] : m_keys)
    {
      XmlNode* boneNode = CreateXmlNode(doc, "node", container);

      boneNode->append_attribute(doc->allocate_attribute(XmlNodeName.data(), boneName.c_str()));

      if constexpr (SERIALIZE_ANIMATION_AS_BINARY)
      {
        WriteAttr(boneNode, doc, "KeyCount", std::to_string(keys.size()));
        size_t keyBufferSize = keys.size() * sizeof(keys[0]);
        char* b64Data        = new char[keyBufferSize * 2];
        bintob64(b64Data, keys.data(), keyBufferSize);
        XmlNode* base64XML = CreateXmlNode(doc, "Base64", boneNode);
        base64XML->value(doc->allocate_string(b64Data));
        SafeDelArray(b64Data);
      }
      else
      {
        for (uint keyIndex = 0; keyIndex < keys.size(); keyIndex++)
        {
          XmlNode* keyNode         = CreateXmlNode(doc, "key", boneNode);
          const Key& key           = keys[keyIndex];

          char* frameIndexValueStr = doc->allocate_string(std::to_string(keyIndex).c_str());
          keyNode->append_attribute(doc->allocate_attribute("frame", frameIndexValueStr));

          WriteVec(CreateXmlNode(doc, "translation", keyNode), doc, key.m_position);

          WriteVec(CreateXmlNode(doc, "scale", keyNode), doc, key.m_scale);

          WriteVec(CreateXmlNode(doc, "rotation", keyNode), doc, key.m_rotation);
        }
      }
    }

    return container;
  }

  XmlNode* Animation::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlAttribute* attr = parent->first_attribute("fps");
    m_fps              = (float) (std::atof(attr->value()));

    attr               = parent->first_attribute("duration");
    m_duration         = (float) (std::atof(attr->value()));

    for (XmlNode* animNode = parent->first_node("node"); animNode; animNode = animNode->next_sibling())
    {
      attr            = animNode->first_attribute(XmlNodeName.data());
      String boneName = attr->value();

      // Serialized as base64
      if (XmlAttribute* keyCountAttr = animNode->first_attribute("KeyCount"))
      {
        uint keyCount = 0;
        ReadAttr(animNode, "KeyCount", keyCount);
        KeyArray& keys = m_keys[boneName];
        keys.resize(keyCount);
        XmlNode* b64Node = animNode->first_node("Base64");
        b64tobin(keys.data(), b64Node->value());
      }
      else
      {
        // Serialized as xml
        for (XmlNode* keyNode = animNode->first_node("key"); keyNode; keyNode = keyNode->next_sibling())
        {
          Key key;
          attr             = keyNode->first_attribute("frame");
          key.m_frame      = std::atoi(attr->value());

          XmlNode* subNode = keyNode->first_node("translation");
          ReadVec(subNode, key.m_position);

          subNode = keyNode->first_node("scale");
          ReadVec(subNode, key.m_scale);

          subNode = keyNode->first_node("rotation");
          ReadVec(subNode, key.m_rotation);

          m_keys[boneName].push_back(key);
        }
      }
    }

    return nullptr;
  }

  void Animation::Init(bool flushClientSideArray) { m_initiated = true; }

  void Animation::UnInit()
  {
    m_initiated = false;
    m_keys.clear();
  }

  void Animation::CopyTo(Resource* other)
  {
    Resource::CopyTo(other);
    Animation* cpy  = static_cast<Animation*>(other);
    cpy->m_keys     = m_keys;
    cpy->m_fps      = m_fps;
    cpy->m_duration = m_duration;
  }

  void Animation::GetNearestKeys(const KeyArray& keys, int& key1, int& key2, float& ratio, float t)
  {
    // Find nearset keys.
    key1  = -1;
    key2  = -1;
    ratio = 0.0f;

    assert(keys.empty() != true && "Animation can't be empty !");

    // Check boundary cases.
    int keySize = static_cast<int>(keys.size());
    if (keySize == 1)
    {
      key1 = 0;
      key2 = 0;
      return;
    }

    // Current time is earliear than earliest time in the animation.
    float boundaryTime = keys.front().m_frame * 1.0f / m_fps;
    if (boundaryTime > t)
    {
      key1 = 0;
      key2 = 1;
      return;
    }

    // Current time is later than the latest time in the animation.
    boundaryTime = keys.back().m_frame * 1.0f / m_fps;
    if (t > boundaryTime)
    {
      key2  = keySize - 1;
      key1  = key2 - 1;
      ratio = 1.0f;
      return;
    }

    // Current time is in between keyframes.
    // Rote interpolation ratio and nearest keys.
    for (int i = 1; i < keySize; i++)
    {
      float keyTime2 = keys[i].m_frame / m_fps;
      float keyTime1 = keys[i - 1].m_frame / m_fps;

      if (t >= keyTime1 && keyTime2 >= t)
      {
        ratio = (t - keyTime1) / (keyTime2 - keyTime1);
        key1  = i - 1;
        key2  = i;
        return;
      }
    }
  }

  AnimRecord::AnimRecord() { m_id = GetHandleManager()->GenerateHandle(); }

  void AnimRecord::Construct(EntityPtr entity, const AnimationPtr& anim)
  {
    m_entity    = entity;
    m_animation = anim;
  }

  AnimRecord::~AnimRecord()
  {
    if (HandleManager* handleMan = GetHandleManager())
    {
      handleMan->ReleaseHandle(m_id);
    }
  }

  AnimationPlayer::~AnimationPlayer() { ClearAnimationData(); }

  void AnimationPlayer::AddRecord(AnimRecordPtr rec)
  {
    int indx = Exist(rec->m_id);
    if (indx != -1)
    {
      return;
    }

    if (m_blendingRecords.find(rec) != m_blendingRecords.end())
    {
      m_blendingRecords.erase(rec);
    }

    for (auto animRecIt = m_records.begin(); animRecIt != m_records.end(); ++animRecIt)
    {
      if (*animRecIt == rec)
      {
        m_records.erase(animRecIt);
        break;
      }
    }

    // Generate animation frame data
    AddAnimationData(rec->m_entity, rec->m_animation);

    m_records.push_back(rec);
  }

  void AnimationPlayer::RemoveRecord(ULongID id)
  {
    int indx = Exist(id);
    if (indx != -1)
    {
      auto it = m_records.begin() + indx;
      m_records.erase(it);

      if (m_blendingRecords.find(*it) != m_blendingRecords.end())
      {
        m_blendingRecords.erase(*it);
      }

      UpdateAnimationData();
    }
  }

  void AnimationPlayer::RemoveRecord(const AnimRecord& rec) { RemoveRecord(rec.m_id); }

  void AnimationPlayer::BlendAnimation(AnimRecordPtr recordToBeBlended,
                                       AnimRecordPtr recordToBlend,
                                       float blendDurationInSec)
  {
    // check if they have same bones
    assert(HaveSameKeys(recordToBeBlended->m_animation->m_keys, recordToBlend->m_animation->m_keys) &&
           "Blend animation is for different skeleton than the animation to blend with!");

    m_blendingRecords[recordToBeBlended]       = recordToBlend;

    recordToBlend->m_blendCurrentDurationInSec = blendDurationInSec;
    recordToBlend->m_blendTotalDurationInSec   = blendDurationInSec;
    recordToBlend->m_blendingActive            = true;
    recordToBlend->m_animRecordToBeBlended     = recordToBeBlended;
  }

  void AnimationPlayer::Update(float deltaTimeSec)
  {
    // Updates all the records in the player and returns true if record needs to be removed.
    auto updateRecordsFn = [&](AnimRecordPtr record) -> bool
    {
      if (record->m_state == AnimRecord::State::Pause)
      {
        return false;
      }

      AnimRecord::State state = record->m_state;
      if (state == AnimRecord::State::Play)
      {
        record->m_currentTime += (deltaTimeSec * record->m_timeMultiplier);
        float duration         = record->m_animation->m_duration;
        if (record->m_loop)
        {
          float leftOver = record->m_currentTime - duration;
          if (leftOver > 0.0)
          {
            record->m_currentTime = leftOver;
          }
        }
        else
        {
          if (record->m_currentTime > duration)
          {
            record->m_state = AnimRecord::State::Stop;
          }
        }

        if (record->m_blendingActive)
        {
          record->m_blendCurrentDurationInSec -= deltaTimeSec * record->m_timeMultiplier;
          if (record->m_blendCurrentDurationInSec < 0.0)
          {
            return true;
          }
        }
      }

      if (state == AnimRecord::State::Rewind)
      {
        record->m_currentTime = 0.0f;
      }

      if (state == AnimRecord::State::Rewind)
      {
        record->m_state = AnimRecord::State::Play;
      }

      return state == AnimRecord::State::Stop;
    };

    // Update all active animation records
    bool anyAnimRecordDeleted = false;
    for (std::vector<AnimRecordPtr>::iterator it = m_records.begin(); it != m_records.end();)
    {
      if (updateRecordsFn(*it))
      {
        // remove record from both blending map and records array

        anyAnimRecordDeleted        = true;

        AnimRecordPtr animRecordPtr = (*it)->m_animRecordToBeBlended;
        if (animRecordPtr != nullptr)
        {
          auto blendingAnimMapIt = m_blendingRecords.find(animRecordPtr);
          if (blendingAnimMapIt != m_blendingRecords.end())
          {
            m_blendingRecords.erase(blendingAnimMapIt);
          }
        }

        if (EntityPtr ntt = (*it)->m_entity.lock())
        {
          if (SkeletonComponentPtr skComp = ntt->GetComponent<SkeletonComponent>())
          {
            skComp->m_animData.currentAnimation = nullptr;
            skComp->m_animData.blendAnimation   = nullptr;
          }
        }

        it = m_records.erase(it);
      }
      else
      {
        ++it;
      }
    }

    // remove unused animation data textures
    if (anyAnimRecordDeleted)
    {
      UpdateAnimationData();
    }

    // Fill skeleton components with anim data
    for (auto it = m_records.begin(); it != m_records.end(); it++)
    {
      AnimRecordPtr record = *it;

      if (EntityPtr ntt = record->m_entity.lock())
      {
        MeshComponentPtr meshComp   = ntt->GetMeshComponent();
        SkeletonComponentPtr skComp = ntt->GetComponent<SkeletonComponent>();
        if (meshComp->GetMeshVal()->IsSkinned() && skComp != nullptr)
        {
          assert(record->m_animation->m_keys.size() > 0);
          KeyArray& keys = (*(record->m_animation->m_keys.begin())).second;
          int key1, key2;
          float ratio;
          record->m_animation->GetNearestKeys(keys, key1, key2, ratio, record->m_currentTime);

          skComp->m_animData.keyFrameCount             = (float) keys.size();
          skComp->m_animData.firstKeyFrame             = (float) key1 / skComp->m_animData.keyFrameCount;
          skComp->m_animData.secondKeyFrame            = (float) key2 / skComp->m_animData.keyFrameCount;
          skComp->m_animData.keyFrameInterpolationTime = ratio;
          skComp->m_animData.currentAnimation          = record->m_animation;

          AnimRecordPtr recordToBlend                  = nullptr;
          auto it                                      = m_blendingRecords.find(record);
          if (it != m_blendingRecords.end())
          {
            recordToBlend = it->second;
          }
          if (recordToBlend != nullptr)
          {
            KeyArray& blendAnimKeys = (*(recordToBlend->m_animation->m_keys.begin())).second;
            recordToBlend->m_animation->GetNearestKeys(blendAnimKeys, key1, key2, ratio, recordToBlend->m_currentTime);

            skComp->m_animData.blendKeyFrameCount = (float) blendAnimKeys.size();
            skComp->m_animData.animationBlendFactor =
                recordToBlend->m_blendCurrentDurationInSec / recordToBlend->m_blendTotalDurationInSec;
            skComp->m_animData.blendFirstKeyFrame             = (float) key1 / skComp->m_animData.blendKeyFrameCount;
            skComp->m_animData.blendSecondKeyFrame            = (float) key2 / skComp->m_animData.blendKeyFrameCount;
            skComp->m_animData.blendKeyFrameInterpolationTime = ratio;
            skComp->m_animData.blendAnimation                 = recordToBlend->m_animation;
          }
          else
          {
            skComp->m_animData.blendAnimation = nullptr;
          }
        }
      }
    }
  }

  int AnimationPlayer::Exist(ULongID id) const
  {
    for (size_t i = 0; i < m_records.size(); i++)
    {
      if (m_records[i]->m_id == id)
      {
        return static_cast<int>(i);
      }
    }

    return -1;
  }

  void AnimationPlayer::AddAnimationData(EntityWeakPtr ntt, AnimationPtr anim)
  {
    if (EntityPtr entity = ntt.lock())
    {
      if (SkeletonComponentPtr skelComp = entity->GetComponent<SkeletonComponent>())
      {
        if (SkeletonPtr skeleton = skelComp->GetSkeletonResourceVal())
        {
          if (m_animTextures.find(std::make_pair(skeleton->GetIdVal(), anim->GetIdVal())) != m_animTextures.end())
          {
            // this animation data already exists
            return;
          }

          DataTexturePtr texture = CreateAnimationDataTexture(skeleton, anim);
          m_animTextures[std::make_pair(skeleton->GetIdVal(), anim->GetIdVal())] = texture;
        }
      }
    }
  }

  void AnimationPlayer::UpdateAnimationData()
  {
    std::map<std::pair<ULongID, ULongID>, DataTexturePtr>::iterator it;
    for (it = m_animTextures.begin(); it != m_animTextures.end();)
    {
      bool found = false;
      for (AnimRecordPtr animRecord : m_records)
      {
        if (EntityPtr entity = animRecord->m_entity.lock())
        {
          if (SkeletonComponentPtr skelComp = entity->GetComponent<SkeletonComponent>())
          {
            if (SkeletonPtr skeleton = skelComp->GetSkeletonResourceVal())
            {
              const ULongID skeletonID = skeleton->GetIdVal();
              const ULongID animID     = animRecord->m_animation->GetIdVal();

              if (it->first.first == skeletonID && it->first.second == animID)
              {
                found = true;
                break;
              }
            }
          }
        }
      }

      if (found)
      {
        ++it;
      }
      else
      {
        it = m_animTextures.erase(it);
      }
    }
  }

  void AnimationPlayer::ClearAnimationData() { m_animTextures.clear(); }

  DataTexturePtr AnimationPlayer::CreateAnimationDataTexture(SkeletonPtr skeleton, AnimationPtr anim)
  {
    if (anim->m_keys.empty())
    {
      return nullptr;
    }

    uint height        = 1024;                               // max number of key frames
    uint width         = (int) skeleton->m_bones.size() * 4; // number of bones * 4 (each element holds a row of matrix)
    uint sizeOfElement = 16 * 4;                             // size of an element in bytes

    char* buffer       = new char[height * width * sizeOfElement];

    uint maxKeyCount   = 0;
    uint keyframeIndex = 0;
    while (true)
    {
      if (keyframeIndex >= height)
      {
        TK_ERR("The maximum number of key frames for animations is 1024!");
        TK_ERR("Animation \"%s\" has more than 1024 key frames.", anim->GetFile().c_str());
        SafeDelArray(buffer);
        return nullptr;
      }

      bool keysframesLeft = false;
      std::vector<std::pair<Node*, uint>> boneNodes;

      // Iterate all bones for the key frame and get node transformations
      for (auto& dBoneIter : skeleton->m_Tpose.boneList)
      {
        const String& name                 = dBoneIter.first;
        DynamicBoneMap::DynamicBone& dBone = dBoneIter.second;

        if (anim->m_keys.find(name) == anim->m_keys.end())
        {
          dBone.node->SetLocalTransforms(Vec3(), Quaternion(), Vec3(1.0f));
          boneNodes.push_back(std::make_pair(dBone.node, dBone.boneIndx));
          continue;
        }

        std::vector<Key>& keys = anim->m_keys[name];
        if (keys.size() <= keyframeIndex)
        {
          continue;
        }
        else
        {
          if (maxKeyCount < keys.size())
          {
            maxKeyCount = (uint) keys.size();
          }

          keysframesLeft = true;

          Key& key       = keys[keyframeIndex];
          dBone.node->SetLocalTransforms(key.m_position, key.m_rotation, key.m_scale);
          boneNodes.push_back(std::make_pair(dBone.node, dBone.boneIndx));
        }
      }

      if (!keysframesLeft)
      {
        break;
      }

      // After getting all node transformations re-calculate dirty nodes transformations
      for (auto& node : boneNodes)
      {
        StaticBone* sBone         = skeleton->m_bones[node.second];

        const Mat4 boneTransform  = node.first->GetTransform(TransformationSpace::TS_WORLD);
        const Mat4 totalTransform = boneTransform * sBone->m_inverseWorldMatrix;

        uint loc                  = ((keyframeIndex * (uint) skeleton->m_bones.size() + node.second) * sizeOfElement);
        memcpy(buffer + loc, &totalTransform, sizeOfElement);
      }

      ++keyframeIndex;
    }

    TextureSettings dataTextureSettings;
    dataTextureSettings.Target         = GraphicTypes::Target2D;
    dataTextureSettings.WarpS          = GraphicTypes::UVClampToEdge;
    dataTextureSettings.WarpT          = GraphicTypes::UVClampToEdge;
    dataTextureSettings.WarpR          = GraphicTypes::UVClampToEdge;
    dataTextureSettings.InternalFormat = GraphicTypes::FormatRGBA32F;
    dataTextureSettings.Format         = GraphicTypes::FormatRGBA;
    dataTextureSettings.Type           = GraphicTypes::TypeFloat;
    DataTexturePtr animDataTexture     = MakeNewPtr<DataTexture>(width, maxKeyCount, dataTextureSettings);
    animDataTexture->Init((void*) buffer);

    SafeDelArray(buffer);

    return animDataTexture;
  }

  AnimationManager::AnimationManager() { m_baseType = Animation::StaticClass(); }

  AnimationManager::~AnimationManager() {}

  bool AnimationManager::CanStore(ClassMeta* Class) { return Class == Animation::StaticClass(); }

} // namespace ToolKit
