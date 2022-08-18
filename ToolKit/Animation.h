#pragma once

/**
* @file Animation.h Header for Animation, AnimationManager, AnimationPlayer
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
    TKResourceType(Animation);

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
    * Sets the Node's transform from the animation based on time.
    * @param node Node to be transformed.
    * @param time Time to fetch the transformation from.
    */
    void GetPose(Node* node, float time);

    /**
    * Sets the Skeleton's transform from the animation based on time.
    * @param skeleton SkeletonPtr to be transformed.
    * 
    */
    void GetPose(const SkeletonPtr& skeleton, float time);

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

    /**
    * Save animation to disk
    */
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;

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
    * A map that holds bone names and their corresponding keys
    * for this animation.
    */
    BoneKeyArrayMap m_keys;
    float m_fps = 30.0f;  //!< Frames to display per second.
    float m_duration = 0.0f;  //!< Duration of the animation.
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
    bool CanStore(ResourceType t) override;
    ResourcePtr CreateLocal(ResourceType type) override;
  };


  /**
  * The class that represents the current state of the animation such as its
  * current time
  */
  class TK_API AnimRecord
  {
   public:
    /**
    * Empty constructor.
    */
    AnimRecord();

    /**
    * Construct an animation record for the enitiy with given animation.
    * @param entity Is the entity to play the animation on.
    * @param anim Is the animation to play for the record.
    */
    AnimRecord(Entity* entity, const AnimationPtr& anim);

   public:
    /**
    * Current time of the animation expressed in seconds.
    */
    float m_currentTime = 0.0f;
    bool m_loop = false;  //!< States if the animation mean to be looped.
    AnimationPtr m_animation;  //!< Animimation to play.
    Entity* m_entity;

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

    State m_state = State::Play;  //!< Current state of the animation.
    ULongID m_id;
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
    void AddRecord(AnimRecord* rec);

    /**
    * Removes the AnimRecord with the given id.
    * @param id Id of the AnimRecord.
    */
    void RemoveRecord(ULongID id);

    /**
    * Update all the records in the player and apply transforms
    * to corresponding entities.
    * @param deltaTimeSec The delta time in seconds for
    * increment for each record.
    */
    void Update(float deltaTimeSec);

    /**
    * Checks if the record exist.
    * @param id Is the id of the AnimRecord to check.
    * @return The index of the record, if it cannot find, returns -1.
    */
    int Exist(ULongID id) const;

   public:
    // Storage for the AnimRecord objects.
    AnimRecordRawPtrArray m_records;
  };

}  // namespace ToolKit
