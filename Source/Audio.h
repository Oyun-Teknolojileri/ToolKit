#pragma once

#include "Resource.h"
#include "ResourceManager.h"
#include "Entity.h"
#include "al.h"
#include "alc.h"

namespace ToolKit
{

  class Audio : public Resource
  {
  public:
    Audio(std::string file);
    ~Audio();

    void Init(bool flushClientSideArray = true);
    void Load();

  public:
    ALuint m_buffer;
  };

  class AudioManager : public ResourceManager<Audio>
  {
  public:
    void Init();
    void Uninit();

  public:
    ALCdevice* m_device = nullptr;
    ALCcontext* m_context = nullptr;
  };

  class AudioSource : public Entity
  {
  public:
    AudioSource();
    ~AudioSource();

    EntityType GetType();
    void AttachAudio(std::shared_ptr<Audio> audio);
    void SetLoop(bool enable);
    void SetVolume(float val);

  public:
    std::shared_ptr<Audio> m_audio;
    ALuint m_source = 0;
  };
  
  class AudioPlayer
  {
  public:
    static void Play(AudioSource* source);
    static void Stop(AudioSource* source);
    static void Rewind(AudioSource* source);
    static void Pause(AudioSource* source);
  };

}