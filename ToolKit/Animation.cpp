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

#include "Animation.h"

#include "AnimationControllerComponent.h"
#include "Common/base64.h"
#include "Entity.h"
#include "FileManager.h"
#include "MathUtil.h"
#include "Node.h"
#include "Skeleton.h"
#include "ToolKit.h"
#include "Util.h"

#include "DebugNew.h"

static constexpr bool SERIALIZE_ANIMATION_AS_BINARY = false;

namespace ToolKit
{

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
    node->m_translation = Interpolate(k1.m_position, k2.m_position, ratio);
    node->m_orientation = glm::slerp(k1.m_rotation, k2.m_rotation, ratio);
    node->m_scale       = Interpolate(k1.m_scale, k2.m_scale, ratio);
    node->SetChildrenDirty();
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
      if (blendTarget != nullptr)
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
            Vec3 entityScale       = skeleton->m_entity->m_node->GetScale();
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

      dBone.node->m_translation = translation;
      dBone.node->m_orientation = orientation;
      dBone.node->m_scale       = scale;

      dBone.node->SetChildrenDirty();
    }
    skeleton->isDirty = true;
  }

  void Animation::GetPose(Node* node, int frame) { GetPose(node, frame * 1.0f / m_fps); }

  void Animation::Load()
  {
    if (m_loaded)
    {
      return;
    }

    XmlFilePtr file = GetFileManager()->GetXmlFile(GetFile());
    XmlDocument doc;
    doc.parse<0>(file->data());

    XmlNode* node = doc.first_node("anim");
    if (node == nullptr)
    {
      return;
    }

    XmlAttribute* attr = node->first_attribute("fps");
    m_fps              = static_cast<float>(std::atof(attr->value()));

    attr               = node->first_attribute("duration");
    m_duration         = static_cast<float>(std::atof(attr->value()));

    for (XmlNode* animNode = node->first_node("node"); animNode; animNode = animNode->next_sibling())
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

    m_loaded = true;
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

  void Animation::Init(bool flushClientSideArray) { m_initiated = true; }

  void Animation::UnInit()
  {
    m_initiated = false;
    m_keys.clear();
  }

  void Animation::Reverse()
  {
    for (auto& keys : m_keys)
    {
      int len = static_cast<int>(m_keys[keys.first].size()) - 1;
      int lim = len / 2;
      for (int i = 0; i < lim; i++)
      {
        std::swap(m_keys[keys.first][i], m_keys[keys.first][len - i]);
        std::swap(m_keys[keys.first][i].m_frame, m_keys[keys.first][len - i].m_frame);
      }
    }
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
      float keyTime2 = keys[i].m_frame * 1.0f / m_fps;
      float keyTime1 = keys[i - 1].m_frame * 1.0f / m_fps;

      if (t >= keyTime1 && keyTime2 >= t)
      {
        ratio = (t - keyTime1) / (keyTime2 - keyTime1);
        key1  = i - 1;
        key2  = i;
        return;
      }
    }
  }

  AnimRecord::AnimRecord() { m_id = GetHandleManager()->GetNextHandle(); }

  AnimRecord::AnimRecord(Entity* entity, const AnimationPtr& anim) : m_entity(entity), m_animation(anim)
  {
    m_id = GetHandleManager()->GetNextHandle();
  }

  void AnimationPlayer::AddRecord(AnimRecord* rec)
  {
    int indx = Exist(rec->m_id);
    if (indx != -1)
    {
      return;
    }

    m_records.push_back(rec);
  }

  void AnimationPlayer::RemoveRecord(ULongID id)
  {
    int indx = Exist(id);
    if (indx != -1)
    {
      m_records.erase(m_records.begin() + indx);
    }
  }

  void AnimationPlayer::RemoveRecord(const AnimRecord& rec) { RemoveRecord(rec.m_id); }

  void AnimationPlayer::Update(float deltaTimeSec)
  {
    // Updates all the records in the player and returns true if record needs to be removed.
    auto updateRecordsFn = [&](AnimRecord* record) -> bool
    {
      if (record->m_state == AnimRecord::State::Pause)
      {
        return false;
      }

      AnimRecord::State state = record->m_state;
      if (state == AnimRecord::State::Play)
      {
        float thisTime = record->m_currentTime + (deltaTimeSec * record->m_timeMultiplier);
        float duration = record->m_animation->m_duration;
        if (record->m_loop)
        {
          if (thisTime > duration)
          {
            record->m_currentTime = 0.0f;
          }
        }
        else
        {
          if (thisTime > duration)
          {
            record->m_state = AnimRecord::State::Stop;
          }
        }
      }
      if (state == AnimRecord::State::Rewind || state == AnimRecord::State::Stop)
      {
        record->m_currentTime = 0;
      }
      else
      {
        record->m_currentTime += deltaTimeSec * record->m_timeMultiplier;
      }
      record->m_entity->SetPose(record->m_animation,
                                record->m_currentTime,
                                record->m_blendTarget.Blend ? &record->m_blendTarget : nullptr);
      if (state == AnimRecord::State::Rewind)
      {
        record->m_state = AnimRecord::State::Play;
      }

      return state == AnimRecord::State::Stop;
    };

    erase_if(m_records, updateRecordsFn);
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

  AnimationManager::AnimationManager() { m_type = ResourceType::Animation; }

  AnimationManager::~AnimationManager() {}

  bool AnimationManager::CanStore(ResourceType t) { return t == ResourceType::Animation; }

  ResourcePtr AnimationManager::CreateLocal(ResourceType type) { return ResourcePtr(new Animation()); }

} // namespace ToolKit
