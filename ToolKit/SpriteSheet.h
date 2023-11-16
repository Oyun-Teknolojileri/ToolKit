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

  struct SpriteEntry
  {
    String name;
    Vec2 offset;
    Rect<int> rectangle;
  };

  class TK_API SpriteSheet : public Resource
  {
   public:
    TKDeclareClass(SpriteSheet, Resource);

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
    std::unordered_map<String, SurfacePtr> m_sprites;
  };

  class TK_API SpriteAnimation : public Entity
  {
   public:
    TKDeclareClass(SpriteAnimation, Entity);

    SpriteAnimation();
    explicit SpriteAnimation(const SpriteSheetPtr& spriteSheet);
    ~SpriteAnimation();

    SurfacePtr GetCurrentSurface();
    void Update(float deltaTime);

   protected:
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

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
    bool CanStore(ClassMeta* Class) override;
    ResourcePtr CreateLocal(ClassMeta* Class) override;
  };

} // namespace ToolKit
