/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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

#include "DebugNew.h"

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
    String file = resource->GetFile();
    bool sane   = !file.empty();
    sane        &= !Exist(file);
    sane        &= CanStore(resource->Class());

    if (sane)
    {
      m_storage[file] = resource;
    }
  }

  String ResourceManager::GetDefaultResource(TKClass* Class) { return String(); }

  bool ResourceManager::Exist(const String& file) { return m_storage.find(file) != m_storage.end(); }

  template <typename T>
  std::shared_ptr<T> ResourceManager::Copy(ResourcePtr source)
  {
    std::shared_ptr<T> resource = MakeNewPtr<T>();
    source->CopyTo(resource.get());
    if (ResourceManager* manager = GetResourceManager(T::StaticClass()))
    {
      manager->Manage(resource);
    }
    return resource;
  }

  template std::shared_ptr<Material> ResourceManager::Copy(ResourcePtr source);
  template std::shared_ptr<Mesh> ResourceManager::Copy(ResourcePtr source);
  template std::shared_ptr<Skeleton> ResourceManager::Copy(ResourcePtr source);

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
