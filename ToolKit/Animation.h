#pragma once

#include <vector>
#include <unordered_map>

#include "Types.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "Entity.h"
#include "Mesh.h"
#include "Node.h"

namespace ToolKit
{

  class Key
  {
   public:
    int m_frame = 0;
    Vec3 m_position;
    Quaternion m_rotation;
    Vec3 m_scale;
  };

  typedef std::vector<Key> KeyArray;
  typedef std::unordered_map<String, KeyArray> BoneKeyArrayMap;
  typedef std::vector<AnimRecord> AnimRecordArray;

  class TK_API Animation : public Resource
  {
   public:
    TKResouceType(Animation)

    Animation();
    explicit Animation(const String& file);
    ~Animation();

    /**
    * Sets the Node's tranform from the animation based on m_currentTime.
    * @node -- Node tobe transformed.
    */
    void GetCurrentPose(Node* node);

    /**
    * Sets the Node's tranform from the animation based on time.
    * @node -- Node tobe transformed.
    * @time -- Time to fetch the transformation from.
    */
    void GetPose(Node* node, float time);
    void GetPose(Node* node, int frame);
    void GetCurrentPose(Skeleton* skeleton);
    float GetDuration();

    void Load() override;
    void Init(bool flushClientSideArray = true) override;
    void UnInit() override;

    void Reverse();  //!< Reverse the animation.

   protected:
    void CopyTo(Resource* other) override;

   private:
    /**
    * Finds nearest keys and interpolation ratio for current time.
    * @keys -- animation key array.
    * @key1 -- output key 1.
    * @key2 -- output key 2.
    * @ratio -- output ratio.
    * @t -- time to search keys for.
    */
    void GetNearestKeys
    (
      const KeyArray& keys,
      int* key1,
      int* key2,
      float* ratio,
      float t
    );

   public:
    enum class State
    {
      Play,
      Pause,
      Rewind,
      Stop
    };

    /**
    * A map that holds bone names and their corresponding keys
    * for this animation.
    */
    BoneKeyArrayMap m_keys;
    float m_fps = 30.0f;

    /**
    * Current time of the animation expressed in seconds.
    */
    float m_currentTime = 0.0f;
    float m_duration = 0.0f;  //!< Duration of the animaton.
    bool m_loop = false;  //!< States if the animation mean to be looped.
    State m_state = State::Play;  //!< Current state of the animation.
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

    /**
    * Checks if record exist.
    * @returns The index of the record, if it cannot find, returns -1.
    */
    int Exist(const AnimRecord& recrod) const;

   public:
      AnimRecordArray m_records;
  };

}  // namespace ToolKit
