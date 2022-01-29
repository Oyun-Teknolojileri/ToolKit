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
#include "Scene.h"
#include "ToolKit.h"
#include "DebugNew.h"

#include <string>

namespace ToolKit
{
  ResourceManager::ResourceManager()
  {
  }

  ResourceManager::~ResourceManager()
  {
    assert(m_storage.size() == 0); // Uninitialize all resources before exit.
  }

  void ResourceManager::Init()
  {
    GetLogger()->Log("Initiating manager " + GetTypeString(m_type));
  }

  void ResourceManager::Uninit()
  {
    GetLogger()->Log("Uninitiating manager " + GetTypeString(m_type));
    m_storage.clear();
  }

  void ResourceManager::Manage(const ResourcePtr& resource)
  {
    bool sane = !resource->m_file.empty();
    sane &= !Exist(resource->m_file);
    sane &= CanStore(resource->GetType());

    if (sane)
    {
      m_storage[resource->m_file] = resource;
    }
  }

  bool ResourceManager::Exist(String file)
  {
    return m_storage.find(file) != m_storage.end();
  }

  ResourcePtr ResourceManager::Remove(const String& file)
  {
    ResourcePtr resource = nullptr;
    auto mapItr = m_storage.find(file);
    if (mapItr  != m_storage.end())
    {
      resource = mapItr->second;
      m_storage.erase(file);
    }

    return resource;
  }

  bool ResourceManager::IsSane(const String& file)
  {
    bool fileCheck = CheckFile(file);
    if (!fileCheck)
    {
      GetLogger()->Log("Missing: " + file);
      assert(fileCheck);
      return false;
    }

    return true;
  }

}
