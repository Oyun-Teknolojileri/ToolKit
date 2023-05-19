#include "Audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "DebugNew.h"

namespace ToolKit
{
  // Audio
  //////////////////////////////////////////////////////////////////////////

  Audio::Audio() {}

  Audio::Audio(const String& file) : Audio() { SetFile(file); }

  Audio::~Audio() { UnInit(); }

  void Audio::Init(bool flushClientSideArray) {}

  void Audio::Load()
  {
    ma_engine* engine = (ma_engine*) GetAudioManager()->m_engine;
    ma_sound* sound   = new ma_sound();
    m_sound           = (void*) sound;
    ma_result result  = ma_sound_init_from_file(engine,
                                               GetFile().c_str(),
                                               0,
                                               nullptr,
                                               nullptr,
                                               sound);
    assert(result == MA_SUCCESS);
  }

  void Audio::UnInit()
  {
    ma_sound* sound = (ma_sound*) m_sound;
    ma_sound_uninit(sound);
    SafeDel(sound);
  }

  // Audio Manager
  //////////////////////////////////////////////////////////////////////////

  AudioManager::AudioManager() { m_type = ResourceType::Audio; }

  AudioManager::~AudioManager() {}

  void AudioManager::Init()
  {
    ResourceManager::Init();
    ma_engine* engine       = new ma_engine();
    ma_engine_config config = ma_engine_config_init();
    config.listenerCount    = MA_ENGINE_MAX_LISTENERS;

    m_engine                = (void*) engine;

    ma_result result        = ma_engine_init(&config, engine);
    assert(result == MA_SUCCESS);
  }

  void AudioManager::Uninit()
  {
    ResourceManager::Uninit();
    ma_engine* engine = (ma_engine*) m_engine;
    ma_engine_uninit(engine);
    SafeDel(engine);
  }

  bool AudioManager::CanStore(ResourceType t)
  {
    return t == ResourceType::Audio;
  }

  ResourcePtr AudioManager::CreateLocal(ResourceType type)
  {
    return ResourcePtr(new Audio());
  }

  // AudioSource
  //////////////////////////////////////////////////////////////////////////

  EntityType AudioSource::GetType() const
  {
    return EntityType::Entity_AudioSource;
  }

  void AudioSource::AttachAudio(std::shared_ptr<Audio> audio)
  {
    m_audio = audio;
  }

  // Setters

  void AudioSource::SetLoop(bool looping)
  {
    ma_sound* sound = (ma_sound*) m_audio->m_sound;
    ma_sound_set_looping(sound, looping ? MA_TRUE : MA_FALSE);
  }

  void AudioSource::SetVolume(float volume)
  {
    ma_sound* sound = (ma_sound*) m_audio->m_sound;
    ma_sound_set_volume(sound, volume);
  }

  void AudioSource::SetSpeed(float speed)
  {
    ma_sound* sound = (ma_sound*) m_audio->m_sound;
    ma_sound_set_pitch(sound, speed);
  }

  void AudioSource::SetPosition(Vec3 pos)
  {
    ma_sound* sound = (ma_sound*) m_audio->m_sound;
    ma_sound_set_position(sound, pos.x, pos.y, pos.z);
  }

  void AudioSource::Play()
  {
    ma_sound* sound = (ma_sound*) m_audio->m_sound;
    ma_sound_start(sound);
  }

  void AudioSource::Stop()
  {
    ma_sound* sound = (ma_sound*) m_audio->m_sound;
    ma_sound_stop(sound);
  }

  // Getters

  bool AudioSource::GetLoop() const
  {
    const ma_sound* sound = (ma_sound*) m_audio->m_sound;
    return ma_sound_is_looping(sound) == MA_TRUE;
  }

  float AudioSource::GetVolume() const
  {
    const ma_sound* sound = (ma_sound*) m_audio->m_sound;
    return ma_sound_get_volume(sound);
  }

  float AudioSource::GetPitch() const
  {
    const ma_sound* sound = (ma_sound*) m_audio->m_sound;
    return ma_sound_get_pitch(sound);
  }

  Vec3 AudioSource::GetPosition() const
  {
    const ma_sound* sound = (ma_sound*) m_audio->m_sound;
    ma_vec3f pos          = ma_sound_get_position(sound);
    return Vec3(pos.x, pos.y, pos.z);
  }

} // namespace ToolKit
