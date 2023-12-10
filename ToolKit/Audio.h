/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Entity.h"
#include "Resource.h"

namespace ToolKit
{

  class TK_API Audio : public Resource
  {
   public:
    TKDeclareClass(Audio, Resource);

    Audio();
    explicit Audio(const String& file);
    ~Audio();

    void Init(bool flushClientSideArray = false) override;
    void Load() override;
    void UnInit() override;

   public:
    void* m_sound = nullptr;
  };

  typedef std::shared_ptr<Audio> AudioPtr;

  class TK_API AudioManager : public ResourceManager
  {
   public:
    AudioManager();
    virtual ~AudioManager();
    void Init() override;
    void Uninit() override;
    void Stop();
    void Start();

    bool CanStore(ClassMeta* Class) override;

   public:
    void* m_engine = nullptr;
  };

  class TK_API AudioSource : public Entity
  {
   public:
    TKDeclareClass(AudioSource, Entity);

    ~AudioSource();
    void AttachAudio(const AudioPtr& audio);

    // seeks the duration'th second of sound
    void Seek(float duration);
    void SetLoop(bool enable);
    void SetVolume(float val);
    void SetPitch(float val);
    void SetPosition(Vec3 pos);

    bool GetLoop() const;
    float GetVolume() const;
    float GetPitch() const;
    Vec3 GetPosition() const;

    bool IsEnd() const;
    bool IsPlaying() const;
    float GetDuration() const;

    void Play();
    void Stop();
    void Rewind();

   protected:
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   public:
    // Audio engine reference.
    void* m_sound = nullptr;

   private:
    // Actual resource reference.
    AudioPtr m_audio = nullptr;
  };
} // namespace ToolKit
