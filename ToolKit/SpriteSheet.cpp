/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "SpriteSheet.h"

#include "Mesh.h"
#include "Node.h"
#include "RapidXml/rapidxml.hpp"
#include "RapidXml/rapidxml_utils.hpp"
#include "Surface.h"
#include "Texture.h"
#include "ToolKit.h"



namespace ToolKit
{

  // SpriteSheet
  //////////////////////////////////////////////////////////////////////////

  TKDefineClass(SpriteSheet, Resource);

  SpriteSheet::SpriteSheet() : m_imageWidth(0), m_imageHeight(0) {}

  SpriteSheet::SpriteSheet(const String& file) : SpriteSheet() { SetFile(file); }

  SpriteSheet::~SpriteSheet() { UnInit(); }

  void SpriteSheet::Load()
  {
    if (m_loaded)
    {
      return;
    }

    if (FetchEntries())
    {
      m_spriteSheet = GetTextureManager()->Create<Texture>(SpritePath(m_imageFile));
      for (const SpriteEntry& entry : m_entries)
      {
        SurfacePtr surface = MakeNewPtr<Surface>();
        surface->Update(m_spriteSheet, entry);
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

    for (const auto& entry : m_sprites)
    {
      entry.second->GetMeshComponent()->Init(flushClientSideArray);
    }

    m_initiated = true;
  }

  void SpriteSheet::UnInit()
  {
    m_sprites.clear();
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
    m_imageFile        = attr->value();
    NormalizePath(m_imageFile);

    attr          = node->first_attribute("w");
    m_imageWidth  = std::atoi(attr->value());

    attr          = node->first_attribute("h");
    m_imageHeight = std::atoi(attr->value());

    for (node = node->first_node("spr"); node; node = node->next_sibling())
    {
      SpriteEntry entry;
      attr                   = node->first_attribute("name");
      entry.name             = attr->value();
      attr                   = node->first_attribute("x");
      entry.rectangle.X      = std::atoi(attr->value());
      attr                   = node->first_attribute("y");
      entry.rectangle.Y      = std::atoi(attr->value());
      attr                   = node->first_attribute("w");
      entry.rectangle.Width  = std::atoi(attr->value());
      attr                   = node->first_attribute("h");
      entry.rectangle.Height = std::atoi(attr->value());

      attr                   = node->first_attribute("px");
      if (attr != nullptr)
      {
        entry.offset.x = static_cast<float>(std::atof(attr->value()));
      }

      attr = node->first_attribute("py");
      if (attr != nullptr)
      {
        entry.offset.y = static_cast<float>(std::atof(attr->value()));
      }

      m_entries.push_back(entry);
    }

    return true;
  }

  // SpriteAnimation
  //////////////////////////////////////////////////////////////////////////

  TKDefineClass(SpriteAnimation, Entity);

  SpriteAnimation::SpriteAnimation() {}

  SpriteAnimation::SpriteAnimation(const SpriteSheetPtr& spriteSheet) { m_sheet = spriteSheet; }

  SpriteAnimation::~SpriteAnimation() {}

  SurfacePtr SpriteAnimation::GetCurrentSurface()
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

  XmlNode* SpriteAnimation::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  SpriteSheetManager::SpriteSheetManager() { m_baseType = SpriteSheet::StaticClass(); }

  SpriteSheetManager::~SpriteSheetManager() {}

  bool SpriteSheetManager::CanStore(ClassMeta* Class) { return Class == SpriteSheet::StaticClass(); }

} // namespace ToolKit
