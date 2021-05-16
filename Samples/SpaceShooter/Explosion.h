#pragma once

#include "ToolKit.h"
#include "Node.h"
#include "Directional.h"

using namespace ToolKit;

class ExplosionManager
{
public:
  ~ExplosionManager()
  {
    for (auto entry : m_sprites)
      SafeDel(entry);
  }

  void SpawnMeteorExplosion(glm::vec2 pos)
  {
    SpriteSheetPtr spriteSheet = Main::GetInstance()->m_spriteSheetMan.Create(SpritePath("explosion.sprites"));
    SpriteAnimation* anim = new SpriteAnimation(spriteSheet);
    anim->m_node->SetTranslation({ pos.x, pos.y, 0.0f }, TransformationSpace::TS_WORLD);
    anim->m_animFps = 30.0f;
    for (int i = 0; i < (int)spriteSheet->m_sprites.size(); i++)
    {
      anim->m_frames.push_back(std::to_string(i));
    }
    m_sprites.push_back(anim);
  }

  void SpawnShipExplosion(glm::vec2 pos)
  {
    SpriteSheetPtr spriteSheet = Main::GetInstance()->m_spriteSheetMan.Create(SpritePath("shipExplosion.sprites"));
    SpriteAnimation* anim = new SpriteAnimation(spriteSheet);
    anim->m_node->SetTranslation({ pos.x, pos.y, 0.0f }, TransformationSpace::TS_WORLD);
    anim->m_animFps = 30.0f;
    for (int i = 0; i < (int)spriteSheet->m_sprites.size(); i++)
    {
      anim->m_frames.push_back(std::to_string(i));
    }

    m_sprites.push_back(anim);
  }

  void Update(float deltaTime)
  {
    for (int i = (int)m_sprites.size() - 1; i > -1; i--)
    {
      if (m_sprites[i]->m_animationStoped)
      {
        SafeDel(m_sprites[i]);
        m_sprites.erase(m_sprites.begin() + i);
      }
      else
      {
        m_sprites[i]->Update(deltaTime);
      }
    }
  }

  std::vector<SpriteAnimation*> m_sprites;
};