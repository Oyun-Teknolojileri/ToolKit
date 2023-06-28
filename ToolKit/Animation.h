/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

/**
 * @file Animation.h Header for Animation, AnimationManager, AnimationPlayer
 * and related structures.
 */

#include "Entity.h"
#include "Node.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "Skeleton.h"
#include "SkeletonComponent.h"
#include "Types.h"

#include <unordered_map>
#include <vector>

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

    /** Reverses the animation. */
    void Reverse();

    /**
     * Save animation to disk
     */
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

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
    float m_currentTime    = 0.0f;
    bool m_loop            = false; //!< States if the animation mean to be looped.
    float m_timeMultiplier = 1.0f;  //!< Speed multiplier for animation.
    AnimationPtr m_animation;       //!< Animation to play.
    Entity* m_entity;
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

   public:
    // Storage for the AnimRecord objects.
    AnimRecordRawPtrArray m_records;
  };
} // namespace ToolKit
