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
