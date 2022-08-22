#include "Animation.h"

#include <vector>
#include <utility>

#include "Entity.h"
#include "Node.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "Util.h"
#include "Skeleton.h"
#include "ToolKit.h"
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

  void Animation::GetPose(const SkeletonPtr& skeleton, float time)
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

      GetNearestKeys(entry->second, key1, key2, ratio, time);

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

  void Animation::GetPose(Node* node, int frame)
  {
    GetPose(node, frame * 1.0f / m_fps);
  }

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
  void Animation::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* container = CreateXmlNode(doc, "anim", parent);

    char* fpsValueStr = doc->allocate_string(std::to_string(m_fps).c_str());
    XmlAttribute* fpsAttrib = doc->allocate_attribute
    (
      "fps",
      fpsValueStr
    );
    container->append_attribute(fpsAttrib);

    char* durationValueStr = doc->allocate_string
    (
      std::to_string(m_duration).c_str()
    );
    XmlAttribute* durAttrib = doc->allocate_attribute
    (
      "duration",
      durationValueStr
    );
    container->append_attribute(durAttrib);

    BoneKeyArrayMap::const_iterator iterator;
    for (auto const& [boneName, keys] : m_keys)
    {
      XmlNode* boneNode = CreateXmlNode(doc, "node", container);

      boneNode->append_attribute
      (
        doc->allocate_attribute("name", boneName.c_str())
      );

      for (uint keyIndex = 0; keyIndex < keys.size(); keyIndex++)
      {
        XmlNode* keyNode = CreateXmlNode(doc, "key", boneNode);
        const Key& key = keys[keyIndex];

        char* frameIndexValueStr = doc->allocate_string
        (
          std::to_string(keyIndex).c_str()
        );
        keyNode->append_attribute
        (
          doc->allocate_attribute("frame", frameIndexValueStr)
        );

        WriteVec
        (
          CreateXmlNode(doc, "translation", keyNode),
          doc,
          key.m_position
        );

        WriteVec
        (
          CreateXmlNode(doc, "scale", keyNode),
          doc,
          key.m_scale
        );

        WriteVec
        (
          CreateXmlNode(doc, "rotation", keyNode),
          doc,
          key.m_rotation
        );
      }
    }
  }

  void Animation::Init(bool flushClientSideArray)
  {
    m_initiated = true;
  }

  void Animation::UnInit()
  {
    m_initiated = false;
    m_keys.clear();
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
    cpy->m_duration = m_duration;
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


  AnimRecord::AnimRecord()
  {
    m_id = GetHandleManager()->GetNextHandle();
  }

  AnimRecord::AnimRecord(Entity* entity, const AnimationPtr& anim)
    : m_entity(entity), m_animation(anim)
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

  void AnimationPlayer::RemoveRecord(const AnimRecord& rec)
  {
    RemoveRecord(rec.m_id);
  }

  void AnimationPlayer::Update(float deltaTimeSec)
  {
    int index = 0;
    std::vector<int> removeList;
    for (AnimRecord* record : m_records)
    {
      if (record->m_state == AnimRecord::State::Pause)
      {
        continue;
      }

      AnimRecord::State state = record->m_state;
      if (state == AnimRecord::State::Play)
      {
        float thisTime = record->m_currentTime + deltaTimeSec;
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

      if
        (
        state == AnimRecord::State::Rewind ||
        state == AnimRecord::State::Stop
        )
      {
        record->m_currentTime = 0;
      }
      else
      {
        record->m_currentTime += deltaTimeSec;
      }

      record->m_entity->SetPose(record->m_animation, record->m_currentTime);

      if (state == AnimRecord::State::Rewind)
      {
        record->m_state = AnimRecord::State::Play;
      }

      if (state == AnimRecord::State::Stop)
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

  int AnimationPlayer::Exist(ULongID id) const
  {
    for (size_t i = 0; i < m_records.size(); i++)
    {
      if (m_records[i]->m_id == id)
      {
        return static_cast<int> (i);
      }
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
