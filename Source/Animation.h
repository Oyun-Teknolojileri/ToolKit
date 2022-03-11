#pragma once

#include "Types.h"
#include "Resource.h"
#include "ResourceManager.h"

namespace ToolKit
{
  class Node;
  class Skeleton;
  class Entity;

  class Key
  {
  public:
    int m_frame = 0;
    Vec3 m_position;
    Quaternion m_rotation;
    Vec3 m_scale;
  };

  class TK_API Animation : public Resource
  {
  public:
    TKResouceType(Animation)

    Animation();
    Animation(String file);
    ~Animation();

    void GetCurrentPose(Node* node); // Set the Node's tranform from the animation based on m_currentTime.
    void GetPose(Node* node, float time); // Set the Node's tranform from the animation based on time.
    void GetPose(Node* node, int frame);
    void GetCurrentPose(Skeleton* skeleton);
    float GetDuration();

    virtual void Load() override;
    virtual void Init(bool flushClientSideArray = true) override;
    virtual void UnInit() override;

    void Reverse(); // Reverse the animation.

  protected:
    virtual void CopyTo(Resource* other) override;

  private:
    void GetNearestKeys(const std::vector<Key>& keys, int& key1, int& key2, float& ratio, float t); // Finds nearest keys and interpolation ratio for current time.

  public:
    enum class State
    {
      Play,
      Pause,
      Rewind,
      Stop
    };

    std::unordered_map<String, std::vector<Key>> m_keys; // Bone - Key.
    float m_fps = 30.0f;
    float m_currentTime = 0.0f; // Seconds
    float m_duration = 0.0f;
    bool m_loop = false;
    State m_state = State::Play;
  };

  class TK_API AnimationManager : public ResourceManager
  {
  public:
    AnimationManager();
    virtual ~AnimationManager();
    virtual bool CanStore(ResourceType t);
    virtual ResourcePtr CreateLocal(ResourceType type);
  };

  class TK_API AnimationPlayer
  {
  public:
    void AddRecord(const AnimRecord& rec);
    void AddRecord(Entity* entity, Animation* anim);
    void RemoveRecord(const AnimRecord& rec);
    void RemoveRecord(Entity* entity, Animation* anim);
    void Update(float deltaTimeSec);
    int Exist(const AnimRecord& recrod) const; // -1 For not exist. Otherwise the record index.

  public:
    std::vector<AnimRecord> m_records;
  };
}
