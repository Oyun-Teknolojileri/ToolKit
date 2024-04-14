/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "ResourceManager.h"

#include "Animation.h"
#include "Audio.h"
#include "Material.h"
#include "Mesh.h"
#include "Scene.h"
#include "Shader.h"
#include "SpriteSheet.h"
#include "Texture.h"
#include "ToolKit.h"
#include "Util.h"

#include <string>



namespace ToolKit
{
  ResourceManager::ResourceManager() {}

  ResourceManager::~ResourceManager()
  {
    assert(m_storage.size() == 0); // Uninitialize all resources before exit.
  }

  void ResourceManager::Init() { GetLogger()->Log("Initiating manager " + m_baseType->Name); }

  void ResourceManager::Uninit()
  {
    GetLogger()->Log("Uninitiating manager " + m_baseType->Name);
    m_storage.clear();
  }

  void ResourceManager::Manage(ResourcePtr resource)
  {
    String file  = resource->GetFile();
    bool sane    = !file.empty();
    sane        &= !Exist(file);
    sane        &= CanStore(resource->Class());

    if (sane)
    {
      m_storage[file] = resource;
    }
  }

  String ResourceManager::GetDefaultResource(ClassMeta* Class) { return String(); }

  bool ResourceManager::Exist(const String& file) { return m_storage.find(file) != m_storage.end(); }

  ResourcePtr ResourceManager::Remove(const String& file)
  {
    ResourcePtr resource = nullptr;
    auto mapItr          = m_storage.find(file);
    if (mapItr != m_storage.end())
    {
      resource = mapItr->second;
      m_storage.erase(file);
    }

    return resource;
  }

} // namespace ToolKit
