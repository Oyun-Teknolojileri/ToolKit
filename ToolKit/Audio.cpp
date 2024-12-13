/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Audio.h"

#include "FileManager.h"

#include "DebugNew.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"

namespace ToolKit
{
  // Audio
  //////////////////////////////////////////

  TKDefineClass(Audio, Resource);

  Audio::Audio() {}

  Audio::Audio(const String& file) : Audio() { SetFile(file); }

  Audio::~Audio() { UnInit(); }

  void Audio::Init(bool flushClientSideArray) {}

  void Audio::Load()
  {
    String path = GetFile();
    m_sound     = GetFileManager()->GetAudioFile(path);

    if (m_sound == nullptr)
    {
      TK_ERR("Cannot load sound file! %s", path.c_str());
    }
  }

  void Audio::UnInit()
  {
    ma_sound* sound = (ma_sound*) m_sound;
    ma_sound_uninit(sound);
    SafeDel(sound);
  }

  // Audio Manager
  //////////////////////////////////////////

  AudioManager::AudioManager() { m_baseType = Audio::StaticClass(); }

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

  void AudioManager::Start() { ma_engine_start((ma_engine*) m_engine); }

  void AudioManager::Stop() { ma_engine_stop((ma_engine*) m_engine); }

  void AudioManager::Uninit()
  {
    ResourceManager::Uninit();
    ma_engine* engine = (ma_engine*) m_engine;
    ma_engine_uninit(engine);
    SafeDel(engine);
  }

  SoundBuffer AudioManager::DecodeFromMemory(ubyte* buffer, uint bufferSize)
  {
    ma_resource_manager* resourceManager = ma_engine_get_resource_manager((ma_engine*) m_engine);
    if (!resourceManager)
    {
      return nullptr;
    }

    // First register the data with the resource manager
    char uniqueName[256];
    sprintf(uniqueName, "memory_sound_%p", buffer); // Create unique name based on buffer address

    ma_resource_manager_register_encoded_data(resourceManager, uniqueName, buffer, bufferSize);

    // Now create the sound using the registered data
    ma_sound* sound  = new ma_sound();
    ma_result result = ma_sound_init_from_file((ma_engine*) m_engine,
                                               uniqueName,
                                               MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION,
                                               nullptr,
                                               nullptr,
                                               sound);

    if (result != MA_SUCCESS)
    {
      TK_ERR("Sound can't be decoded from memory.");
      SafeDel(sound);
      return nullptr;
    }

    return (SoundBuffer) sound;
  }

  SoundBuffer AudioManager::DecodeFromFile(StringView file)
  {
    ma_sound* sound  = new ma_sound();
    ma_result result = ma_sound_init_from_file((ma_engine*) m_engine,
                                               file.data(),
                                               MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION,
                                               nullptr,
                                               nullptr,
                                               sound);

    if (result != MA_SUCCESS)
    {
      TK_ERR("Sound %s can't be decoded.", file);
      SafeDel(sound);
      return nullptr;
    }

    return (SoundBuffer) sound;
  }

  bool AudioManager::CanStore(ClassMeta* Class) { return Class == Audio::StaticClass(); }

  // AudioSource
  //////////////////////////////////////////

  TKDefineClass(AudioSource, Entity);

  AudioSource::~AudioSource()
  {
    if (m_sound != nullptr)
    {
      ma_sound_uninit((ma_sound*) m_sound);
      SafeDel(m_sound);
    }
  }

  void AudioSource::AttachAudio(const AudioPtr& audio)
  {
    m_audio                     = audio;

    ma_engine* engine           = (ma_engine*) GetAudioManager()->m_engine;
    const ma_sound* sourceSound = (const ma_sound*) audio->m_sound;

    ma_sound* copySound         = new ma_sound();
    memset(copySound, 0, sizeof(ma_sound));

    ma_sound_init_copy(engine, sourceSound, MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, copySound);
    m_sound = (void*) copySound;
  }

  // Setters

  void AudioSource::SetLoop(bool looping) { ma_sound_set_looping((ma_sound*) m_sound, looping ? MA_TRUE : MA_FALSE); }

  void AudioSource::SetVolume(float volume) { ma_sound_set_volume((ma_sound*) m_sound, volume); }

  void AudioSource::Seek(float duration)
  {
    ma_uint32 sampRate;
    ma_sound* sound = (ma_sound*) m_sound;
    ma_sound_get_data_format(sound, nullptr, nullptr, &sampRate, nullptr, 0);
    ma_sound_seek_to_pcm_frame(sound, (ma_uint64) (duration * sampRate));
  }

  void AudioSource::SetPitch(float val) { ma_sound_set_pitch((ma_sound*) m_sound, val); }

  void AudioSource::SetPosition(Vec3 pos) { ma_sound_set_position((ma_sound*) m_sound, pos.x, pos.y, pos.z); }

  void AudioSource::Play() { ma_sound_start((ma_sound*) m_sound); }

  void AudioSource::Stop() { ma_sound_stop((ma_sound*) m_sound); }

  XmlNode* AudioSource::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  // Getters

  bool AudioSource::IsEnd() const { return ma_sound_at_end((const ma_sound*) m_sound) == MA_TRUE; }

  bool AudioSource::IsPlaying() const { return ma_sound_is_playing((const ma_sound*) m_sound) == MA_TRUE; }

  float AudioSource::GetDuration() const
  {
    float duration  = 0.0f;
    // ma_sound_get_length_in_seconds takes a non-const pointer (not sure why).
    // To get around this, we need a const_cast on the this pointer.
    ma_sound* sound = (ma_sound*) const_cast<void*>(m_sound);
    ma_sound_get_length_in_seconds(sound, &duration);
    return duration;
  }

  bool AudioSource::GetLoop() const { return ma_sound_is_looping((const ma_sound*) m_sound) == MA_TRUE; }

  float AudioSource::GetVolume() const { return ma_sound_get_volume((const ma_sound*) m_sound); }

  float AudioSource::GetPitch() const { return ma_sound_get_pitch((const ma_sound*) m_sound); }

  Vec3 AudioSource::GetPosition() const
  {
    ma_vec3f pos = ma_sound_get_position((const ma_sound*) m_sound);
    return Vec3(pos.x, pos.y, pos.z);
  }

} // namespace ToolKit
