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
    ResourcePtr CreateLocal(ClassMeta* Class) override;

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
