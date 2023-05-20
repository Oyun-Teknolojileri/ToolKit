#pragma once

#include "Entity.h"
#include "Resource.h"
#include "ResourceManager.h"

namespace ToolKit
{

  class TK_API Audio : public Resource
  {
   public:
    TKResourceType(Audio)

    Audio();
    explicit Audio(const String& file);
    ~Audio();

    void Init(bool flushClientSideArray = false) override;
    void Load() override;
    void UnInit() override;

   public:
    void* m_sound;
    uint m_buffer;
  };

  typedef std::shared_ptr<Audio> AudioPtr;

  class TK_API AudioManager : public ResourceManager
  {
   public:
    AudioManager();
    virtual ~AudioManager();
    void Init() override;
    void Uninit() override;
    bool CanStore(ResourceType t) override;
    ResourcePtr CreateLocal(ResourceType type) override;

   public:
    void* m_engine = nullptr;
  };

  class TK_API AudioSource : public Entity
  {
   public:
    EntityType GetType() const override;
    void AttachAudio(std::shared_ptr<Audio> audio);
    void SetLoop(bool enable);
    void SetVolume(float val);
    void SetPitch(float val);
    void SetPosition(Vec3 pos);

    bool GetLoop() const;
    float GetVolume() const;
    float GetPitch() const;
    Vec3 GetPosition() const;

    void Play();
    void Stop();
    void Rewind();

   public:
    std::shared_ptr<Audio> m_audio;
    uint m_source = 0;
  };
} // namespace ToolKit
