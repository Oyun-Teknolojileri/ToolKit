#pragma once

#include <memory>

#include "Resource.h"
#include "ResourceManager.h"
#include "Entity.h"

namespace ToolKit
{

  class TK_API Audio : public Resource
  {
   public:
    TKResourceType(Audio)

      Audio();
    explicit Audio(String file);
    ~Audio();

    void Init(bool flushClientSideArray = true) override;
    void Load() override;
    void UnInit() override;

   public:
    uint m_buffer;
  };

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
    void* m_device = nullptr;
    void* m_context = nullptr;
  };

  class TK_API AudioSource : public Entity
  {
   public:
    AudioSource();
    ~AudioSource();

    EntityType GetType() const override;
    void AttachAudio(std::shared_ptr<Audio> audio);
    void SetLoop(bool enable);
    void SetVolume(float val);

   public:
    std::shared_ptr<Audio> m_audio;
    uint m_source = 0;
  };

  class TK_API AudioPlayer
  {
   public:
    static void Play(AudioSource* source);
    static void Stop(AudioSource* source);
    static void Rewind(AudioSource* source);
    static void Pause(AudioSource* source);
  };

}  // namespace ToolKit
