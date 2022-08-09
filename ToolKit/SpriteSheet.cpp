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
    : m_imageWidth(0), m_imageHeight(0)
  {
  }

  SpriteSheet::SpriteSheet(String file)
    : SpriteSheet()
  {
    SetFile(file);
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
      m_spriteSheet = GetTextureManager()->Create<Texture>
      (
        SpritePath(m_imageFile)
      );
      for (const SpriteEntry& entry : m_entries)
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
      entry.second->GetMeshComponent()->Init(flushClientSideArray);
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
    XmlFile file(GetFile().c_str());
    XmlDocument doc;
    doc.parse<0>(file.data());

    XmlNode* node = doc.first_node("img");
    if (node == nullptr)
    {
      return false;
    }

    XmlAttribute* attr = node->first_attribute("name");
    m_imageFile = attr->value();
    NormalizePath(m_imageFile);

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
      {
        entry.offset.x = static_cast<float> (std::atof(attr->value()));
      }

      attr = node->first_attribute("py");
      if (attr != nullptr)
      {
        entry.offset.y = static_cast<float> (std::atof(attr->value()));
      }

      m_entries.push_back(entry);
    }

    return true;
  }

  SpriteAnimation::SpriteAnimation()
  {
  }

  SpriteAnimation::SpriteAnimation(const SpriteSheetPtr& spriteSheet)
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
        for (size_t curr = 0; curr < m_frames.size(); curr++)
        {
          if (m_currentFrame.compare(m_frames[curr]) == 0)
          {
            if (curr + 1 < m_frames.size())
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

  SpriteSheetManager::SpriteSheetManager()
  {
    m_type = ResourceType::SpriteSheet;
  }

  SpriteSheetManager::~SpriteSheetManager()
  {
  }

  bool SpriteSheetManager::CanStore(ResourceType t)
  {
    return t == ResourceType::SpriteSheet;
  }

  ResourcePtr SpriteSheetManager::CreateLocal(ResourceType type)
  {
    return ResourcePtr(new SpriteSheet());
  }

}  // namespace ToolKit

