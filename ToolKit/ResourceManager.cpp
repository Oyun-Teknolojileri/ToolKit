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

  void ResourceManager::Init() { GetLogger()->Log("Initiating manager " + GetTypeString(m_type)); }

  void ResourceManager::Uninit()
  {
    GetLogger()->Log("Uninitiating manager " + GetTypeString(m_type));
    m_storage.clear();
  }

  void ResourceManager::Manage(const ResourcePtr& resource)
  {
    String file = resource->GetFile();
    bool sane   = !file.empty();
    sane        &= !Exist(file);
    sane        &= CanStore(resource->GetType());

    if (sane)
    {
      m_storage[file] = resource;
    }
  }

  String ResourceManager::GetDefaultResource(ResourceType type) { return String(); }

  bool ResourceManager::Exist(String file) { return m_storage.find(file) != m_storage.end(); }

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

} // namespace ToolKit
