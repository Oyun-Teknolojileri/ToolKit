/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

/**
 * @file Animation.h Header for Animation, AnimationManager, AnimationPlayer
 * and related structures.
 */

#include "DataTexture.h"
#include "Resource.h"
#include "SkeletonComponent.h"
#include "Types.h"

#include <map>

namespace ToolKit
{
  /*
   * Blending to next Animation configration for AnimRecord class.
   */
  struct BlendTarget
  {
    Animation* TargetAnim = nullptr; //!< Animation to Blend.
    float OverlapTime     = 1.0f;    //!< How early animation will start blending.
    String RootBone;                 //!< Root bone of animation nodes for offsetting.
    Vec3 TranslationOffset;          //!< Transform offset of target animation.
    Quaternion OrientationOffset;    //!< Orientation offset of target animation.
    bool Blend = false;              //!< States if the blending is active for the track.
  };

  /**
   * A transformation key that is part of an Animation resource.
   */
  struct Key
  {
    int m_frame = 0;       //!< Order / Frame of the key.
    Vec3 m_position;       //!< Position of the transform.
    Quaternion m_rotation; //!< Rotation of the transform.
    Vec3 m_scale;          //!< Scale of the transform.
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
    TKDeclareClass(Animation, Resource);

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
    virtual ~Animation();

    /**
     * Sets the Node's transform from the animation based on time.
     * @param node Node to be transformed.
     * @param time Time to fetch the transformation from.
     */
    void GetPose(Node* node, float time);

    /**
     * Sets the Skeleton's transform from the animation based on time.
     * @param skeleton SkeletonPtr to be transformed.
     */
    void GetPose(const SkeletonComponentPtr& skeleton, float time, BlendTarget* blendTarget = nullptr);

    /**
     * Sets the Node's transform from the animation based on frame.
     * @see GetPose(Node* node, float time)
     */
    void GetPose(Node* node, int frame);

    void Load() override; //!< Loads the animation data from file.

    /**
     * Set the resource to initiated state.
     * @param flushClientSideArray unused.
     */
    void Init(bool flushClientSideArray = false) override;

    /**
     * Set the resource to uninitiated state and removes the keys.
     */
    void UnInit() override;

    /**
     * Finds nearest keys and interpolation ratio for current time.
     * @param keys animation key array.
     * @param key1 output key 1.
     * @param key2 output key 2.
     * @param ratio output ratio.
     * @param t time to search keys for.
     */
    void GetNearestKeys(const KeyArray& keys, int& key1, int& key2, float& ratio, float t);

   protected:
    void CopyTo(Resource* other) override;

    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

   public:
    /**
     * A map that holds bone names and their corresponding keys
     * for this animation.
     */
    BoneKeyArrayMap m_keys;
    float m_fps      = 30.0f; //!< Frames to display per second.
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
    bool CanStore(ClassMeta* Class) override;
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
    void Construct(EntityPtr entity, const AnimationPtr& anim);

    ~AnimRecord();

   public:
    /**
     * Current time of the animation expressed in seconds.
     */
    float m_currentTime    = 0.0f;
    bool m_loop            = false; //!< States if the animation mean to be looped.
    float m_timeMultiplier = 1.0f;  //!< Speed multiplier for animation.
    AnimationPtr m_animation;       //!< Animation to play.
    EntityWeakPtr m_entity;
    BlendTarget m_blendTarget;

    /**
     * Enums that represent's the current state of the Animation in the
     * AnimationPlayer.
     */
    enum class State
    {
      Play,   //!< Animation is playing.
      Pause,  //!< Animation is paused.
      Rewind, //!< Animation will be rewind by the AnimationPlayer.
      Stop    //!< Stopped playing and will be removed from the AnimationPlayer.
    };

    State m_state = State::Play; //!< Current state of the animation.
    ULongID m_id;
  };

  /**
   * The class that is responsible playing animation records
   * and updating transformations of the corresponding Entities.
   */
  class TK_API AnimationPlayer
  {
   public:
    ~AnimationPlayer();

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
     * Removes the given AnimRecord.
     * @param rec Record to remove.
     */
    void RemoveRecord(const AnimRecord& rec);

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

    inline AnimationDataTexturePtr GetAnimationDataTexture(ULongID skelID, ULongID animID)
    {
      const std::pair<ULongID, ULongID> p = std::make_pair(skelID, animID);
      if (m_animTextures.find(p) != m_animTextures.end())
      {
        return m_animTextures[p];
      }
      else
      {
        return nullptr;
      }
    }

   private:
    /**
     * Add data texture of animation for skeleton
     */
    void AddAnimationData(EntityWeakPtr ntt, AnimationPtr anim);
    /**
     * Removes the unnecessary data textures
     */
    void UpdateAnimationData();
    /**
     * Clears the animation data textures
     */
    void ClearAnimationData();
    /**
     * Creates and returns animation data texture for given skeleton and animation
     */
    AnimationDataTexturePtr CreateAnimationDataTexture(SkeletonPtr skeleton, AnimationPtr anim);

   public:
    // Storage for the AnimRecord objects.
    AnimRecordRawPtrArray m_records;

   private:
    // Storage for animation data (skeleton id - animation id pair)
    std::map<std::pair<ULongID, ULongID>, AnimationDataTexturePtr> m_animTextures;
  };
} // namespace ToolKit
