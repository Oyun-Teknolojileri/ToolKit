#include "stdafx.h"
#include "Animation.h"
#include "Entity.h"
#include "Node.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "Util.h"
#include "Skeleton.h"
#include "DebugNew.h"

namespace ToolKit
{

  Animation::Animation()
  {
    m_type = ResourceType::Animation;
  }

  Animation::Animation(String file)
    : Animation()
  {
    m_file = file;
  }

  Animation::~Animation()
  {
    UnInit();
  }

  void Animation::GetCurrentPose(Node* node)
  {
    if (m_keys.empty())
    {
      return;
    }

    float ratio;
    int key1, key2;
    std::vector<Key>& keys = m_keys.begin()->second;
    GetNearestKeys(keys, key1, key2, ratio);

    if ((int)keys.size() <= key1)
    {
      return;
    }

    if ((int)keys.size() <= key2)
    {
      return;
    }

    Key k1 = keys[key1];
    Key k2 = keys[key2];
    node->m_translation = Interpolate(k1.m_position, k2.m_position, ratio);
    node->m_orientation = glm::slerp(k1.m_rotation, k2.m_rotation, ratio);
    node->m_scale = Interpolate(k1.m_scale, k2.m_scale, ratio), TransformationSpace::TS_LOCAL;
    node->SetChildrenDirty();
  }

  void Animation::GetCurrentPose(Skeleton* skeleton)
  {
    if (m_keys.empty())
      return;

    float ratio;
    int key1, key2;
    for (auto bone : skeleton->m_bones)
    {
      auto entry = m_keys.find(bone->m_name);
      if (entry == m_keys.end())
      {
        continue;
      }

      GetNearestKeys(entry->second, key1, key2, ratio);

      // Sanity checks
      int keySize = (int)entry->second.size();
      if (keySize <= key1 || keySize <= key2)
      {
        continue;
      }

      if (key1 == -1 || key2 == -1)
      {
        continue;
      }

      Key k1 = entry->second[key1];
      Key k2 = entry->second[key2];
      bone->m_node->m_translation = Interpolate(k1.m_position, k2.m_position, ratio);
      bone->m_node->m_orientation = glm::slerp(k1.m_rotation, k2.m_rotation, ratio);
      bone->m_node->m_scale = Interpolate(k1.m_scale, k2.m_scale, ratio), TransformationSpace::TS_LOCAL;
      bone->m_node->SetChildrenDirty();
    }
  }

  float Animation::GetDuration()
  {
    return m_duration;
  }

  void Animation::Load()
  {
    XmlFile file(m_file.c_str());
    XmlDocument doc;
    doc.parse<0>(file.data());

    XmlNode* node = doc.first_node("anim");
    if (node == nullptr)
    {
      return;
    }

    XmlAttribute* attr = node->first_attribute("fps");
    m_fps = (float)std::atof(attr->value());

    attr = node->first_attribute("duration");
    m_duration = (float)std::atof(attr->value());

    for (XmlNode* animNode = node->first_node("node"); animNode; animNode = animNode->next_sibling())
    {
      attr = animNode->first_attribute("name");
      String boneName = attr->value();

      for (XmlNode* keyNode = animNode->first_node("key"); keyNode; keyNode = keyNode->next_sibling())
      {
        Key key;
        attr = keyNode->first_attribute("frame");
        key.m_frame = std::atoi(attr->value());

        XmlNode* subNode = keyNode->first_node("translation");
        ReadVec(subNode, key.m_position);

        subNode = keyNode->first_node("scale");
        ReadVec(subNode, key.m_scale);

        subNode = keyNode->first_node("rotation");
        ReadVec(subNode, key.m_rotation);

        m_keys[boneName].push_back(key);
      }
    }

    m_loaded = true;
  }

  void Animation::Init(bool flushClientSideArray)
  {
  }

  void Animation::UnInit()
  {
    m_initiated = false;
  }

  Animation* Animation::GetCopy()
  {
    return new Animation(*this);
  }

  void Animation::GetNearestKeys(const std::vector<Key>& keys, int& key1, int& key2, float& ratio)
  {
    // Find nearset keys.
    key1 = -1;
    key2 = -1;
    ratio = 0;

    for (int i = 1; i < (int)keys.size(); i++)
    {
      float keyTime2 = keys[i].m_frame * 1.0f / m_fps;
      float keyTime1 = keys[i - 1].m_frame * 1.0f / m_fps;

      if (m_currentTime >= keyTime1 && keyTime2 >= m_currentTime)
      {
        ratio = (m_currentTime - keyTime1) / (keyTime2 - keyTime1);
        key1 = i - 1;
        key2 = i;
        return;
      }
    }
  }

  void AnimationPlayer::AddRecord(Entity* entity, Animation* anim)
  {
    if (Exist({ entity, anim }) == -1)
    {
      m_records.push_back(AnimRecord(entity, anim));
    }
  }

  void AnimationPlayer::RemoveRecord(Entity* entity, Animation* anim)
  {
    int indx = Exist({ entity, anim });
    if (indx != -1)
    {
      m_records.erase(m_records.begin() + indx);
    }
  }

  void AnimationPlayer::Update(float deltaTimeSec)
  {
    int index = 0;
    std::vector<int> removeList;
    for (auto record : m_records)
    {
      if (record.second->m_state == Animation::State::Pause)
      {
        continue;
      }

      Animation::State state = record.second->m_state;
      if (state == Animation::State::Play)
      {
        float thisTime = record.second->m_currentTime + deltaTimeSec;
        float duration = record.second->GetDuration();

        if (record.second->m_loop)
        {
          if (thisTime > duration)
          {
            record.second->m_currentTime = 0.0f;
          }
        }
        else
        {
          if (thisTime > duration)
          {
            record.second->m_state = Animation::State::Stop;
          }
        }
      }

      if (state == Animation::State::Rewind || state == Animation::State::Stop)
      {
        record.second->m_currentTime = 0;
      }
      else
      {
        record.second->m_currentTime += deltaTimeSec;
      }

      record.first->SetPose(record.second);

      if (state == Animation::State::Rewind)
      {
        record.second->m_state = Animation::State::Play;
      }

      if (state == Animation::State::Stop)
      {
        removeList.push_back(index);
      }
      index++;
    }

    std::reverse(removeList.begin(), removeList.end());
    for (int i = 0; i < (int)removeList.size(); i++)
    {
      m_records.erase(m_records.begin() + removeList[i]);
    }
  }

  int AnimationPlayer::Exist(const AnimRecord& record) const
  {
    int index = 0;
    for (const AnimRecord& rec : m_records)
    {
      if (rec.first == record.first && rec.second == record.second)
      {
        return index;
      }
      index++;
    }

    return -1;
  }

  AnimationManager::AnimationManager()
  {
    m_type = ResourceType::Animation;
  }

  AnimationManager::~AnimationManager()
  {
  }

}
