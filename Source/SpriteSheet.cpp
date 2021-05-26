#include "stdafx.h"
#include "ToolKit.h"
#include "SpriteSheet.h"
#include "Mesh.h"
#include "Node.h"
#include "Surface.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "DebugNew.h"

namespace ToolKit
{

  SpriteSheet::SpriteSheet()
  {
  }

  SpriteSheet::SpriteSheet(String file)
  {
    m_file = file;
  }

  SpriteSheet::~SpriteSheet()
  {
    UnInit();
  }

  void SpriteSheet::Load()
  {
    if (m_loaded)
    {
      return;
    }

    if (FetchEntries())
    {
      m_spriteSheet = GetTextureManager()->Create(SpritePath(m_imageFile));
      for (auto entry : m_entries)
      {
        Surface* surface = new Surface(m_spriteSheet, entry);
        m_sprites[entry.name] = surface;
      }
    }

    m_loaded = true;
  }

  void SpriteSheet::Init(bool flushClientSideArray)
  {
    if (m_initiated)
    {
      return;
    }

    for (auto entry : m_sprites)
    {
      entry.second->m_mesh->Init(flushClientSideArray);
    }

    m_initiated = true;
  }

  void SpriteSheet::UnInit()
  {
    for (auto entry : m_sprites)
    {
      SafeDel(entry.second);
    }
    m_initiated = false;
  }

  bool SpriteSheet::FetchEntries()
  {
    XmlFile file(m_file.c_str());
    XmlDocument doc;
    doc.parse<0>(file.data());

    XmlNode* node = doc.first_node("img");
    if (node == nullptr)
    {
      return false;
    }

    XmlAttribute* attr = node->first_attribute("name");
    m_imageFile = attr->value();

    attr = node->first_attribute("w");
    m_imageWidth = std::atoi(attr->value());

    attr = node->first_attribute("h");
    m_imageHeight = std::atoi(attr->value());

    for (node = node->first_node("spr"); node; node = node->next_sibling())
    {
      SpriteEntry entry;
      attr = node->first_attribute("name");
      entry.name = attr->value();
      attr = node->first_attribute("x");
      entry.rectangle.x = std::atoi(attr->value());
      attr = node->first_attribute("y");
      entry.rectangle.y = std::atoi(attr->value());
      attr = node->first_attribute("w");
      entry.rectangle.width = std::atoi(attr->value());
      attr = node->first_attribute("h");
      entry.rectangle.height = std::atoi(attr->value());

      attr = node->first_attribute("px");
      if (attr != nullptr)
        entry.offset.x = (float)std::atof(attr->value());

      attr = node->first_attribute("py");
      if (attr != nullptr)
        entry.offset.y = (float)std::atof(attr->value());

      m_entries.push_back(entry);
    }

    return true;
  }

  SpriteAnimation::SpriteAnimation()
  {
  }

  SpriteAnimation::SpriteAnimation(SpriteSheetPtr spriteSheet)
  {
    m_sheet = spriteSheet;
  }

  SpriteAnimation::~SpriteAnimation()
  {
  }

  EntityType SpriteAnimation::GetType() const
  {
    return EntityType::Entity_SpriteAnim;
  }

  Surface* SpriteAnimation::GetCurrentSurface()
  {
    auto res = m_sheet->m_sprites.find(m_currentFrame);
    if (res == m_sheet->m_sprites.end())
    {
      if (m_frames.empty())
      {
        return m_sheet->m_sprites.begin()->second;
      }
      else
      {
        return m_sheet->m_sprites[m_frames.front()];
      }
    }

    return m_sheet->m_sprites[m_currentFrame];
  }

  void SpriteAnimation::Update(float deltaTime)
  {
    if (m_sheet == nullptr)
    {
      return;
    }

    if (m_animationStoped)
    {
      return;
    }

    if (m_currentTime > m_prevTime + 1.0f / m_animFps)
    {
      if (m_currentFrame.empty())
      {
        m_currentFrame = m_frames.front();
      }
      else
      {
        for (int curr = 0; curr < (int)m_frames.size(); curr++)
        {
          if (m_currentFrame.compare(m_frames[curr]) == 0)
          {
            if (curr + 1 < (int)m_frames.size())
            {
              m_currentFrame = m_frames[curr + 1];
              break;
            }
            else
            {
              if (m_looping)
              {
                m_currentFrame = m_frames.front();
              }
              else
              {
                m_animationStoped = true;
              }
              break;
            }
          }
        }
      }

      m_prevTime = m_currentTime;
    }

    m_currentTime += deltaTime;
  }

}
