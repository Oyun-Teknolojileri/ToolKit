#include "Animation.h"

#include <vector>
#include <utility>

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
  }

  Animation::Animation(const String& file)
    : Animation()
  {
    SetFile(file);
  }

  Animation::~Animation()
  {
    UnInit();
  }

  void Animation::GetCurrentPose(Node* node)
  {
    GetPose(node, m_currentTime);
  }

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

    int keySize = static_cast<int> (keys.size());
    if (keys.size() <= key1 || key1 == -1)
    {
      return;
    }

    if (keys.size() <= key2 || key2 == -1)
    {
      return;
    }

    Key k1 = keys[key1];
    Key k2 = keys[key2];
    node->m_translation = Interpolate(k1.m_position, k2.m_position, ratio);
    node->m_orientation = glm::slerp(k1.m_rotation, k2.m_rotation, ratio);
    node->m_scale = Interpolate(k1.m_scale, k2.m_scale, ratio);
    node->SetChildrenDirty();
  }

  void Animation::GetPose(Node* node, int frame)
  {
    GetPose(node, frame * 1.0f / m_fps);
  }

  void Animation::GetCurrentPose(Skeleton* skeleton)
  {
    if (m_keys.empty())
    {
      return;
    }

    float ratio;
    int key1, key2;
    for (Bone* bone : skeleton->m_bones)
    {
      auto entry = m_keys.find(bone->m_name);
      if (entry == m_keys.end())
      {
        continue;
      }

      GetNearestKeys(entry->second, key1, key2, ratio, m_currentTime);

      // Sanity checks
      int keySize = static_cast<int> (entry->second.size());
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
      bone->m_node->m_translation = Interpolate
      (
        k1.m_position,
        k2.m_position,
        ratio
      );

      bone->m_node->m_orientation = glm::slerp
      (
        k1.m_rotation,
        k2.m_rotation,
        ratio
      );

      bone->m_node->m_scale = Interpolate(k1.m_scale, k2.m_scale, ratio);
      bone->m_node->SetChildrenDirty();
    }
  }

  float Animation::GetDuration()
  {
    return m_duration;
  }

  void Animation::Load()
  {
    if (m_loaded)
    {
      return;
    }

    XmlFile file(GetFile().c_str());
    XmlDocument doc;
    doc.parse<0>(file.data());

    XmlNode* node = doc.first_node("anim");
    if (node == nullptr)
    {
      return;
    }

    XmlAttribute* attr = node->first_attribute("fps");
    m_fps = static_cast<float> (std::atof(attr->value()));

    attr = node->first_attribute("duration");
    m_duration = static_cast<float> (std::atof(attr->value()));

    for
    (
      XmlNode* animNode = node->first_node("node");
      animNode;
      animNode = animNode->next_sibling()
    )
    {
      attr = animNode->first_attribute("name");
      String boneName = attr->value();

      for
      (
        XmlNode* keyNode = animNode->first_node("key");
        keyNode;
        keyNode = keyNode->next_sibling()
      )
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

  void Animation::Reverse()
  {
    for (auto& keys : m_keys)
    {
      int len = static_cast<int> (m_keys[keys.first].size()) - 1;
      int lim = len / 2;
      for (int i = 0; i < lim; i++)
      {
        std::swap
        (
          m_keys[keys.first][i],
          m_keys[keys.first][len - i]
        );
        std::swap
        (
          m_keys[keys.first][i].m_frame,
          m_keys[keys.first][len - i].m_frame
        );
      }
    }
  }

  void Animation::CopyTo(Resource* other)
  {
    Resource::CopyTo(other);
    Animation* cpy = static_cast<Animation*> (other);
    cpy->m_keys = m_keys;
    cpy->m_fps = m_fps;
    cpy->m_currentTime = m_currentTime;
    cpy->m_duration = m_duration;
    cpy->m_loop = m_loop;
    cpy->m_state = m_state;
  }

  void Animation::GetNearestKeys
  (
    const KeyArray& keys,
    int& key1,
    int& key2,
    float& ratio,
    float t
  )
  {
    // Find nearset keys.
    key1 = -1;
    key2 = -1;
    ratio = 0.0f;

    assert(keys.empty() != true && "Animation can't be empty !");

    // Check boundary cases.
    int keySize = static_cast<int> (keys.size());
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
      key2 = keySize - 1;
      key1 = key2 - 1;
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
        key1 = i - 1;
        key2 = i;
        return;
      }
    }
  }

  void AnimationPlayer::AddRecord(const AnimRecord& rec)
  {
    if (Exist(rec) == -1)
    {
      m_records.push_back(rec);
    }
  }

  void AnimationPlayer::AddRecord(Entity* entity, Animation* anim)
  {
    AddRecord({ entity, anim });
  }

  void AnimationPlayer::RemoveRecord(const AnimRecord& rec)
  {
    int indx = Exist(rec);
    if (indx != -1)
    {
      m_records.erase(m_records.begin() + indx);
    }
  }

  void AnimationPlayer::RemoveRecord(Entity* entity, Animation* anim)
  {
    RemoveRecord({ entity, anim });
  }

  void AnimationPlayer::Update(float deltaTimeSec)
  {
    int index = 0;
    std::vector<int> removeList;
    for (const AnimRecord& record : m_records)
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
    for (size_t i = 0; i < removeList.size(); i++)
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

  bool AnimationManager::CanStore(ResourceType t)
  {
    return t == ResourceType::Animation;
  }

  ResourcePtr AnimationManager::CreateLocal(ResourceType type)
  {
    return ResourcePtr(new Animation());
  }

}  // namespace ToolKit
