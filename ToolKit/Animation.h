#pragma once

/**
* @file Header for Animation, AnimationManager, AnimationPlayer
* and related structures.
*/

#include <vector>
#include <unordered_map>

#include "Types.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "Entity.h"
#include "Node.h"
#include "Skeleton.h"

/**
* Base name space for all the ToolKit functionalities.
*/
namespace ToolKit
{

  /**
  * A transformation key that is part of an Animation resource.
  */
  struct Key
  {
    int m_frame = 0;  //!< Order / Frame of the key.
    Vec3 m_position;  //!< Position of the transform.
    Quaternion m_rotation;  //!< Rotation of the transform.
    Vec3 m_scale;  //!< Scale of the transform.
  };

  typedef std::vector<Key> KeyArray;
  typedef std::unordered_map<String, KeyArray> BoneKeyArrayMap;
  typedef std::vector<AnimRecord> AnimRecordArray;

  /**
  * The class that represents animations which can be played with
  * AnimationPlayer. Alter's Entity Node transforms or Skeleton / Bone
  * transforms to apply the animation to the corresponding Entity.
  */
  class TK_API Animation : public Resource
  {
   public:
    /**
    * Auto generated code for type information.
    */
    TKResouceType(Animation);

    /**
    * Empty constructor.
    */
    Animation();

    /**
    * Constructs an Animation object from the file.
    * @param file Disk file to load the resource from.
    */
    explicit Animation(const String& file);

    /**
    * Uninitiate and frees the memory.
    */
    ~Animation();

    /**
    * Sets the Node's transform from the animation based on m_currentTime.
    * @param node Node to be transformed.
    */
    void GetCurrentPose(Node* node);

    /**
    * Sets the Skeleton's transform from the animation based on m_currentTime.
    * @param skeleton Skeleton to be transformed.
    */
    void GetCurrentPose(Skeleton* skeleton);

    /**
    * Sets the Node's transform from the animation based on time.
    * @param node Node to be transformed.
    * @param time Time to fetch the transformation from.
    */
    void GetPose(Node* node, float time);

    /**
    * Sets the Node's transform from the animation based on frame.
    * @see GetPose(Node* node, float time)
    */
    void GetPose(Node* node, int frame);

    void Load() override;  //!< Loads the animation data from file.

    /**
    * Set the resource to initiated state.
    * @param flushClientSideArray unused.
    */
    void Init(bool flushClientSideArray = true) override;

    /**
    * Set the resource to uninitiated state and removes the keys.
    */
    void UnInit() override;

    /** Reverses the animation. */
    void Reverse();

   protected:
    void CopyTo(Resource* other) override;

   private:
    /**
    * Finds nearest keys and interpolation ratio for current time.
    * @param keys animation key array.
    * @param key1 output key 1.
    * @param key2 output key 2.
    * @param ratio output ratio.
    * @param t time to search keys for.
    */
    void GetNearestKeys
    (
      const KeyArray& keys,
      int& key1,
      int& key2,
      float& ratio,
      float t
    );

   public:
    /**
    * Enums that represent's the current state of the Animation in the
    * AnimationPlayer.
    */
    enum class State
    {
      Play,  //!< Animation is playing.
      Pause,  //!< Animation is paused.
      Rewind,  //!< Animation will be rewind by the AnimationPlayer.
      Stop  //!< Stopped playing and will be removed from the AnimationPlayer.
    };

    /**
    * A map that holds bone names and their corresponding keys
    * for this animation.
    */
    BoneKeyArrayMap m_keys;
    float m_fps = 30.0f;  //!< Frames to display per second.

    /**
    * Current time of the animation expressed in seconds.
    */
    float m_currentTime = 0.0f;
    float m_duration = 0.0f;  //!< Duration of the animation.
    bool m_loop = false;  //!< States if the animation mean to be looped.
    State m_state = State::Play;  //!< Current state of the animation.
  };

  /**
  * The class responsible for managing
  * the life time and storing initial instances of the Animation resources.
  */
  class TK_API AnimationManager : public ResourceManager
  {
   public:
    AnimationManager();
    virtual ~AnimationManager();
    virtual bool CanStore(ResourceType t);
    virtual ResourcePtr CreateLocal(ResourceType type);
  };

  /**
  * The class that is responsible playing animation records
  * and updating transformations of the corresponding Entities.
  */
  class TK_API AnimationPlayer
  {
   public:
    /**
    * Adds a record to the player.
    * @param rec AnimRecord data.
    */
    void AddRecord(const AnimRecord& rec);

    /**
    * Construct an AnimRecord and adds it to the player.
    * @param entity Entity to associate animations with.
    * @param anim Animation to apply.
    */
    void AddRecord(Entity* entity, Animation* anim);

    /**
    * Removes a record from the player.
    * @param rec AnimRecord data to remove.
    */
    void RemoveRecord(const AnimRecord& rec);

    /**
    * Constructs an AnimRecord and removes the corresponding record
    * from the player if it exist.
    * @param entity Entity to associate animations with.
    * @param anim Animation to apply.
    */
    void RemoveRecord(Entity* entity, Animation* anim);

    /**
    * Update all the records in the player and apply transforms
    * to corresponding entities.
    * @param deltaTimeSec The delta time in seconds for
    * increment for each record.
    */
    void Update(float deltaTimeSec);

    /**
    * Checks if the record exist.
    * @return The index of the record, if it cannot find, returns -1.
    */
    int Exist(const AnimRecord& recrod) const;

   public:
    AnimRecordArray m_records;  //!< Storage for the AnimRecord objects.
  };

}  // namespace ToolKit
