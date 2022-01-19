#pragma once

#include "MathUtil.h"
#include "Drawable.h"
#include "Resource.h"
#include "ResourceManager.h"

namespace ToolKit
{

  class Texture;
  class Vertex;
  class Surface;

  struct SpriteEntry
  {
    String name;
    Vec2 offset;
    Rect<int> rectangle;
  };

  class TK_API SpriteSheet : public Resource
  {
  public:
    TKResouceType(SpriteSheet)

    SpriteSheet();
    SpriteSheet(String file);
    ~SpriteSheet();

    virtual void Load() override;
    virtual void Init(bool flushClientSideArray = true) override;
    virtual void UnInit() override;

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

  class TK_API SpriteAnimation : public Drawable
  {
  public:
    SpriteAnimation();
    SpriteAnimation(SpriteSheetPtr spriteSheet);
    ~SpriteAnimation();

    virtual EntityType GetType() const;
    Surface* GetCurrentSurface();
    void Update(float deltaTime);

  public:
    float m_animFps = 23.4f;
    bool m_looping = false;
    bool m_animationStoped = false;
    StringArray m_frames;
    SpriteSheetPtr m_sheet;
    String m_currentFrame;

  private:
    float m_currentTime = 0.0f; // Seconds
    float m_prevTime = 0.0f;
  };

  class TK_API SpriteSheetManager : public ResourceManager
  {
  public:
    SpriteSheetManager();
    virtual ~SpriteSheetManager();
    virtual bool CanStore(ResourceType t);
    virtual ResourcePtr CreateLocal(ResourceType type);
  };

}