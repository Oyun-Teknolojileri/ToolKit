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
    String path       = GetFile();
    assert(CheckFile(path) && "audio file is not exist!");
    ma_result result = ma_sound_init_from_file(engine, path.c_str(), 0, nullptr, nullptr, sound);
    if (result != MA_SUCCESS)
    {
      GetLogger()->WritePlatformConsole(LogType::Memo, "cannot load sound file! %s", path.c_str());
      assert(0);
    }
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

  bool AudioManager::CanStore(ResourceType t) { return t == ResourceType::Audio; }

  ResourcePtr AudioManager::CreateLocal(ResourceType type) { return ResourcePtr(new Audio()); }

  // AudioSource
  //////////////////////////////////////////////////////////////////////////

  EntityType AudioSource::GetType() const { return EntityType::Entity_AudioSource; }

  void AudioSource::AttachAudio(std::shared_ptr<Audio> audio) { m_audio = audio; }

  // Setters

  void AudioSource::SetLoop(bool looping)
  {
    ma_sound_set_looping((ma_sound*) m_audio->m_sound, looping ? MA_TRUE : MA_FALSE);
  }

  void AudioSource::SetVolume(float volume) { ma_sound_set_volume((ma_sound*) m_audio->m_sound, volume); }

  void AudioSource::SetPitch(float val) { ma_sound_set_pitch((ma_sound*) m_audio->m_sound, val); }

  void AudioSource::SetPosition(Vec3 pos) { ma_sound_set_position((ma_sound*) m_audio->m_sound, pos.x, pos.y, pos.z); }

  void AudioSource::Play() { ma_sound_start((ma_sound*) m_audio->m_sound); }

  void AudioSource::Stop() { ma_sound_stop((ma_sound*) m_audio->m_sound); }

  // Getters

  bool AudioSource::GetLoop() const { return ma_sound_is_looping((ma_sound*) m_audio->m_sound) == MA_TRUE; }

  float AudioSource::GetVolume() const { return ma_sound_get_volume((ma_sound*) m_audio->m_sound); }

  float AudioSource::GetPitch() const { return ma_sound_get_pitch((ma_sound*) m_audio->m_sound); }

  Vec3 AudioSource::GetPosition() const
  {
    ma_vec3f pos          = ma_sound_get_position((ma_sound*) m_audio->m_sound);
    return Vec3(pos.x, pos.y, pos.z);
  }

} // namespace ToolKit
