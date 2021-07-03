#include "stdafx.h"
#include "ResourceManager.h"
#include "Util.h"
#include "Animation.h"
#include "Audio.h"
#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "SpriteSheet.h"
#include "Texture.h"
#include "DebugNew.h"

#include <string>

namespace ToolKit
{
  void ResourceManager::Init()
  {
    Logger::GetInstance()->Log("Initiating manager " + std::to_string((int)m_type));
  }

  void ResourceManager::Uninit()
  {
    Logger::GetInstance()->Log("Uninitiating manager " + std::to_string((int)m_type));
    m_storage.clear();
  }

  ResourceManager::~ResourceManager()
  {
    assert(m_storage.size() == 0); // Uninitialize all resources before exit.
  }

  ResourcePtr ResourceManager::Create(const String& file)
  {
    return Create(file, m_type);
  }

  ResourcePtr ResourceManager::Create(const String& file, ResourceType type)
  {
    if (!Exist(file))
    {
      bool fileCheck = CheckFile(file);
      if (!fileCheck)
      {
        Logger::GetInstance()->Log("Missing: " + file);
        assert(fileCheck);
        return nullptr;
      }

      ResourcePtr resource;
      switch (type)
      {
      case ResourceType::Animation:
        resource = std::make_shared<Animation>(file);
        break;
      case ResourceType::Audio:
        resource = std::make_shared<Audio>(file);
        break;
      case ResourceType::Material:
        resource = std::make_shared<Material>(file);
        break;
      case ResourceType::Mesh:
        resource = std::make_shared<Mesh>(file);
        break;
      case ResourceType::Shader:
        resource = std::make_shared<Shader>(file);
        break;
      case ResourceType::SkinMesh:
        resource = std::make_shared<SkinMesh>(file);
        break;
      case ResourceType::SpriteSheet:
        resource = std::make_shared<SpriteSheet>(file);
        break;
      case ResourceType::Texture:
        resource = std::make_shared<Texture>(file);
        break;
      case ResourceType::CubeMap:
        resource = std::make_shared<CubeMap>(file);
        break;
      case ResourceType::Base:
      default:
        assert(false && "Undefined type is not allowed.");
        break;
      }
      resource->Load();
      m_storage[file] = resource;
    }

    return m_storage[file];
  }

  bool ResourceManager::Exist(String file)
  {
    return m_storage.find(file) != m_storage.end();
  }
}
