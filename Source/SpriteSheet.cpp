#include "stdafx.h"
#include "ToolKit.h"
#include "SpriteSheet.h"
#include "Mesh.h"
#include "Node.h"
#include "Surface.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "DebugNew.h"

ToolKit::SpriteSheet::SpriteSheet()
{
}

ToolKit::SpriteSheet::SpriteSheet(std::string file)
{
  m_file = file;
}

ToolKit::SpriteSheet::~SpriteSheet()
{
	UnInit();
}

void ToolKit::SpriteSheet::Load()
{
  if (m_loaded)
    return;

  if (FetchEntries())
  {
    m_spriteSheet = Main::GetInstance()->m_textureMan.Create(ToolKit::SpritePath(m_imageFile));
    for (auto entry : m_entries)
    {
      Surface* surface = new Surface(m_spriteSheet, CreateQuat(entry));
      m_sprites[entry.name] = surface;
    }
  }

  m_loaded = true;
}

void ToolKit::SpriteSheet::Init(bool flushClientSideArray)
{
  if (m_initiated)
    return;

  for (auto entry : m_sprites)
    entry.second->Init(flushClientSideArray);

  m_initiated = true;
}

void ToolKit::SpriteSheet::UnInit()
{
	for (auto entry : m_sprites)
		SafeDel(entry.second);
	m_initiated = false;
}

bool ToolKit::SpriteSheet::FetchEntries()
{
  rapidxml::file<> file(m_file.c_str());
  rapidxml::xml_document<> doc;
  doc.parse<0>(file.data());

  rapidxml::xml_node<>* node = doc.first_node("img");
  if (node == nullptr)
    return false;

  rapidxml::xml_attribute<> *attr = node->first_attribute("name");
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

std::vector<ToolKit::Vertex> ToolKit::SpriteSheet::CreateQuat(SpriteEntry val)
{
  Rect<float> textureRect;
  textureRect.x = (float)val.rectangle.x / (float)m_imageWidth;
  textureRect.height = ((float)val.rectangle.height / (float)m_imageHeight);
  textureRect.y = 1.0f - ((float)val.rectangle.y / (float)m_imageHeight) - textureRect.height;
  textureRect.width = (float)val.rectangle.width / (float)m_imageWidth;

  float depth = 0.0f;
  float width = (float)val.rectangle.width;
  float height = (float)val.rectangle.height;
  glm::vec2 absOffset = glm::vec2(val.offset.x * val.rectangle.width, val.offset.y * val.rectangle.height);

  std::vector<Vertex> vertices;
  vertices.resize(6);
  vertices[0].pos = glm::vec3(-absOffset.x, -absOffset.y, depth);
  vertices[0].tex = glm::vec2(textureRect.x, 1.0f - textureRect.y);
  vertices[1].pos = glm::vec3(width - absOffset.x, -absOffset.y, depth);
  vertices[1].tex = glm::vec2(textureRect.x + textureRect.width, 1.0f - textureRect.y);
  vertices[2].pos = glm::vec3(-absOffset.x, height - absOffset.y, depth);
  vertices[2].tex = glm::vec2(textureRect.x, 1.0f - (textureRect.y + textureRect.height));

  vertices[3].pos = glm::vec3(width - absOffset.x, -absOffset.y, depth);
  vertices[3].tex = glm::vec2(textureRect.x + textureRect.width, 1.0f - textureRect.y);
  vertices[4].pos = glm::vec3(width - absOffset.x, height - absOffset.y, depth);
  vertices[4].tex = glm::vec2(textureRect.x + textureRect.width, 1.0f - (textureRect.y + textureRect.height));
  vertices[5].pos = glm::vec3(-absOffset.x, height - absOffset.y, depth);
  vertices[5].tex = glm::vec2(textureRect.x, 1.0f - (textureRect.y + textureRect.height));

  return vertices;
}

ToolKit::SpriteAnimation::SpriteAnimation()
{
}

ToolKit::SpriteAnimation::SpriteAnimation(std::shared_ptr<SpriteSheet> spriteSheet)
{
  m_sheet = spriteSheet;
}

ToolKit::SpriteAnimation::~SpriteAnimation()
{
}

ToolKit::EntityType ToolKit::SpriteAnimation::GetType()
{
  return EntityType::Entity_SpriteAnim;
}

ToolKit::Surface* ToolKit::SpriteAnimation::GetCurrentSurface()
{
  auto res = m_sheet->m_sprites.find(m_currentFrame);
  if (res == m_sheet->m_sprites.end())
  {
    if (m_frames.empty())
      return m_sheet->m_sprites.begin()->second;
    else
      return m_sheet->m_sprites[m_frames.front()];
  }

  return m_sheet->m_sprites[m_currentFrame];
}

void ToolKit::SpriteAnimation::Update(float deltaTime)
{
  if (m_sheet == nullptr)
    return;

  if (m_animationStoped)
    return;

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
              m_currentFrame = m_frames.front();
            else
              m_animationStoped = true;
            break;
          }
        }
      }
    }

    m_prevTime = m_currentTime;
  }

  m_currentTime += deltaTime;
}
