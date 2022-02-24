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

  String ResourceManager::GetDefaultResource(ResourceType type)
  {
    return String();
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

  void ResourceManager::Report(const char* msg, ...)
  {
    va_list args;
    va_start(args, msg);

    static char buff[2048];
    vsprintf(buff, msg, args);
    
    if (m_reporterFn)
    {
      m_reporterFn(buff);
    }
    else
    {
      GetLogger()->Log(buff);
    }

    va_end(args);
  }

}
