#pragma once

#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "Resource.h"
#include "ResourceManager.h"
#include <vector>
#include <unordered_map>

namespace ToolKit
{
  class Key
  {
  public:
    int m_frame = 0;
    glm::vec3 m_position;
    glm::quat m_rotation;
    glm::vec3 m_scale;
  };

  class Skeleton;
  class Node;

  class Animation : public Resource
  {
  public:
    Animation();
    Animation(std::string file);
    ~Animation();

    void GetCurrentPose(Node* node); // interpolate keys based on time.
    void GetCurrentPose(Skeleton* skeleton);
    float GetDuration();

    virtual void Load() override;
    virtual void Init(bool flushClientSideArray = true) override;
		virtual void UnInit() override;
    virtual Animation* GetCopy() override;
    

  private:
    void GetNearestKeys(const std::vector<Key>& keys, int& key1, int& key2, float& ratio); // Finds nearest keys and ratio to current time.

  public:
    enum State
    {
      Play,
      Pause,
      Rewind,
      Stop
    };

    std::unordered_map<std::string, std::vector<Key> > m_keys;
    float m_fps = 30.0f;
    float m_currentTime = 0.0f; // Seconds
    float m_duration = 0.0f;
    bool m_loop = false;
    State m_state = Play;
  };

  class AnimationManager : public ResourceManager <Animation>
  {
  };

  class Entity;
  class AnimationPlayer
  {
  public:
    void AddRecord(Entity* entity, Animation* anim);
    void RemoveRecord(Entity* entity, Animation* anim);
    void Update(float deltaTimeSec);

  public:
    std::vector<std::pair<Entity*, Animation*>> m_records;
  };
}
