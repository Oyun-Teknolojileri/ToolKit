#include "stdafx.h"
#include "Animation.h"
#include "Entity.h"
#include "Node.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "Util.h"
#include "Skeleton.h"

ToolKit::Animation::Animation()
{
}

ToolKit::Animation::Animation(std::string file)
{
  m_file = file;
}

ToolKit::Animation::~Animation()
{
	UnInit();
}

void ToolKit::Animation::GetCurrentPose(Node* node)
{
  if (m_keys.empty())
    return;

  float ratio;
  int key1, key2;
  std::vector<Key>& keys = m_keys.begin()->second;
  GetNearestKeys(keys, key1, key2, ratio);

  if ((int)keys.size() <= key1)
    return;

  if ((int)keys.size() <= key2)
    return;

  Key k1 = keys[key1];
  Key k2 = keys[key2];
  node->m_translation = Interpolate(k1.m_position, k2.m_position, ratio);
  node->m_orientation = glm::slerp(k1.m_rotation, k2.m_rotation, ratio);
  node->m_scale = Interpolate(k1.m_scale, k2.m_scale, ratio);
}

void ToolKit::Animation::GetCurrentPose(Skeleton* skeleton)
{
  if (m_keys.empty())
    return;

  float ratio;
  int key1, key2;
  for (auto bone : skeleton->m_bones)
  {
    auto entry = m_keys.find(bone->m_name);
    if (entry == m_keys.end())
      continue;
    
    GetNearestKeys(entry->second, key1, key2, ratio);

    // Sanity checks
    int keySize = (int)entry->second.size();
    if (keySize <= key1 || keySize <= key2)
      continue;

    if (key1 == -1 || key2 == -1)
      continue;

    Key k1 = entry->second[key1];
    Key k2 = entry->second[key2];
    bone->m_node->m_translation = Interpolate(k1.m_position, k2.m_position, ratio);
    bone->m_node->m_orientation = glm::slerp(k1.m_rotation, k2.m_rotation, ratio);
    bone->m_node->m_scale = Interpolate(k1.m_scale, k2.m_scale, ratio);
  }
}

float ToolKit::Animation::GetDuration()
{
  return m_duration;
}

void ToolKit::Animation::Load()
{
  rapidxml::file<> file(m_file.c_str());
  rapidxml::xml_document<> doc;
  doc.parse<0>(file.data());

  rapidxml::xml_node<>* node = doc.first_node("anim");
  if (node == nullptr)
    return;

  rapidxml::xml_attribute<>* attr = node->first_attribute("fps");
  m_fps = (float)std::atof(attr->value());

  attr = node->first_attribute("duration");
  m_duration = (float)std::atof(attr->value());

  for (rapidxml::xml_node<>* animNode = node->first_node("node"); animNode; animNode = animNode->next_sibling())
  {
    attr = animNode->first_attribute("name");
    std::string boneName = attr->value();

    for (rapidxml::xml_node<>* keyNode = animNode->first_node("key"); keyNode; keyNode = keyNode->next_sibling())
    {
      Key key;
      attr = keyNode->first_attribute("frame");
      key.m_frame = std::atoi(attr->value());

      rapidxml::xml_node<>* subNode = keyNode->first_node("translation");
      ExtractXYZFromNode(subNode, key.m_position);

      subNode = keyNode->first_node("scale");
      ExtractXYZFromNode(subNode, key.m_scale);

      subNode = keyNode->first_node("rotation");
      ExtractQuatFromNode(subNode, key.m_rotation);

      m_keys[boneName].push_back(key);
    }
  }

  m_loaded = true;
}

void ToolKit::Animation::Init(bool flushClientSideArray)
{
}

void ToolKit::Animation::UnInit()
{
	m_initiated = false;
}

ToolKit::Animation* ToolKit::Animation::GetCopy()
{
  return new Animation(*this);
}

void ToolKit::Animation::GetNearestKeys(const std::vector<Key>& keys, int& key1, int& key2, float& ratio)
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

void ToolKit::AnimationPlayer::AddRecord(Entity* entity, Animation* anim)
{
  m_records.push_back(std::pair<Entity*, Animation*>(entity, anim));
}

void ToolKit::AnimationPlayer::RemoveRecord(Entity* entity, Animation* anim)
{
  int index = 0;
  for (auto& record : m_records)
  {
    if (record.first == entity && record.second == anim)
    {
      m_records.erase(m_records.begin() + index);
      return;
    }
    index++;
  }
}

void ToolKit::AnimationPlayer::Update(float deltaTimeSec)
{
  int index = 0;
  std::vector<int> removeList;
  for (auto record : m_records)
  {
    if (record.second->m_state == Animation::Pause)
      continue;

    Animation::State state = record.second->m_state;
    if (state == Animation::Play)
    {
      float thisTime = record.second->m_currentTime + deltaTimeSec;
      float duration = record.second->GetDuration();

      if (record.second->m_loop)
      {
        if (thisTime > duration)
          record.second->m_currentTime = 0.0f;
      }
      else
      {
        if (thisTime > duration)
          record.second->m_state = Animation::Stop;
      }
    }

    if (state == Animation::Rewind || state == Animation::Stop)
      record.second->m_currentTime = 0;
    else
      record.second->m_currentTime += deltaTimeSec;

    record.first->SetPose(record.second);

    if (state == Animation::Rewind)
      record.second->m_state = Animation::Play;

    if (state == Animation::Stop)
      removeList.push_back(index);
    index++;
  }

  std::reverse(removeList.begin(), removeList.end());
  for (int i = 0; i < (int)removeList.size(); i++)
    m_records.erase(m_records.begin() + removeList[i]);
}
