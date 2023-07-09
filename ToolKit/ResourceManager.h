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

#pragma once

#include "Types.h"
#include "Util.h"

namespace ToolKit
{

  enum class ResourceType
  {
    Base,
    Animation,
    Audio,
    Material,
    Mesh,
    Shader,
    SkinMesh,
    SpriteSheet,
    Texture,
    CubeMap,
    Hdri,
    RenderTarget,
    Scene,
    Skeleton,
    DataTexture
  };

  class TK_API ResourceManager
  {
   public:
    ResourceManager();
    virtual ~ResourceManager();
    virtual void Init();
    virtual void Uninit();
    virtual void Manage(const ResourcePtr& resource);
    virtual bool CanStore(ResourceType t) = 0;
    virtual String GetDefaultResource(ResourceType type);

    ResourceManager(const ResourceManager&) = delete;
    void operator=(const ResourceManager&)  = delete;

    template <typename T>
    std::shared_ptr<T> Create(const String& file)
    {
      if (!Exist(file))
      {
        std::shared_ptr<T> resource = std::static_pointer_cast<T>(CreateLocal(T::GetTypeStatic()));
        if (!CheckFile(file))
        {
          String def = GetDefaultResource(T::GetTypeStatic());
          if (!CheckFile(def))
          {
            Report("%s", file.c_str());
            assert(0 && "No default resource!");
            return nullptr;
          }

          String rel = GetRelativeResourcePath(file);
          Report("%s is missing. Using default resource.", rel.c_str());
          resource->SetFile(def);
          resource->_missingFile = file;
        }
        else
        {
          resource->SetFile(file);
        }

        resource->Load();
        m_storage[file] = resource;
      }

      return std::reinterpret_pointer_cast<T>(m_storage[file]);
    }

    bool Exist(const String& file);
    ResourcePtr Remove(const String& file);
    virtual ResourcePtr CreateLocal(ResourceType type) = 0;

   protected:
    void Report(const char* msg, ...);

   public:
    // Log callback, if provided messages passed to callback.
    std::function<void(const String&)> m_reporterFn = nullptr;
    std::unordered_map<String, ResourcePtr> m_storage;
    ResourceType m_type = ResourceType::Base;
  };

} // namespace ToolKit
