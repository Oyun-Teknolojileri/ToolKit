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

#include "Drawable.h"
#include "MathUtil.h"
#include "Resource.h"
#include "ResourceManager.h"

#include <unordered_map>
#include <vector>

namespace ToolKit
{

  struct SpriteEntry
  {
    String name;
    Vec2 offset;
    Rect<int> rectangle;
  };

  class TK_API SpriteSheet : public Resource
  {
   public:
    TKResourceType(SpriteSheet)

    SpriteSheet();
    explicit SpriteSheet(const String& file);
    ~SpriteSheet();

    void Load() override;
    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;

   private:
    bool FetchEntries();

   public:
    TexturePtr m_spriteSheet;
    std::vector<SpriteEntry> m_entries;
    String m_imageFile;
    int m_imageWidth;
    int m_imageHeight;

   public:
    std::unordered_map<String, Surface*> m_sprites;
  };

  class TK_API SpriteAnimation : public Entity
  {
   public:
    SpriteAnimation();
    explicit SpriteAnimation(const SpriteSheetPtr& spriteSheet);
    ~SpriteAnimation();

    virtual EntityType GetType() const;
    Surface* GetCurrentSurface();
    void Update(float deltaTime);

   public:
    float m_animFps        = 23.4f;
    bool m_looping         = false;
    bool m_animationStoped = false;
    StringArray m_frames;
    SpriteSheetPtr m_sheet;
    String m_currentFrame;

   private:
    float m_currentTime = 0.0f; // Seconds
    float m_prevTime    = 0.0f;
  };

  class TK_API SpriteSheetManager : public ResourceManager
  {
   public:
    SpriteSheetManager();
    virtual ~SpriteSheetManager();
    bool CanStore(ResourceType t) override;
    ResourcePtr CreateLocal(ResourceType type) override;
  };

} // namespace ToolKit
