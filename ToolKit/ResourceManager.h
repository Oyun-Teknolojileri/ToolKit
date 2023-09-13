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

#include "Logger.h"
#include "ObjectFactory.h"
#include "Resource.h"
#include "ToolKit.h"
#include "Types.h"
#include "Util.h"

namespace ToolKit
{

  TK_API extern class ResourceManager* GetResourceManager(TKClass* Class);

  class TK_API ResourceManager
  {
   public:
    ResourceManager();
    virtual ~ResourceManager();
    virtual void Init();
    virtual void Uninit();
    virtual void Manage(ResourcePtr resource);
    virtual bool CanStore(TKClass* Class) = 0;
    virtual String GetDefaultResource(TKClass* Class);

    ResourceManager(const ResourceManager&) = delete;
    void operator=(const ResourceManager&)  = delete;

    template <typename T>
    std::shared_ptr<T> Create(const String& file)
    {
        if (!Exist(file)) 
        {
            ResourcePtr resource = MakeNewPtr<T>();
            if (!CheckFile(file)) 
            {
                String def = GetDefaultResource(T::StaticClass());
                if (!CheckFile(def)) 
                {
                    GetLogger()->Log(LogType::Error, "No default for Class %s", T::StaticClass()->Name.c_str());
                    assert(0 && "No default resource!");
                    return nullptr;
                }
        
                String rel = GetRelativeResourcePath(file);
                GetLogger()->Log(LogType::Warning, "File: %s is missing. Using default resource.", rel.c_str());
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
        return tk_reinterpret_pointer_cast<T>(m_storage[file]);
    }

    template <typename T>
    std::shared_ptr<T> Copy(ResourcePtr source)
    {
      std::shared_ptr<T> resource = MakeNewPtr<T>();
      source->CopyTo(resource.get());
      if (ResourceManager* manager = GetResourceManager(T::StaticClass()))
      {
        manager->Manage(resource);
      }
      return resource;
    }

    bool Exist(const String& file);
    ResourcePtr Remove(const String& file);
    virtual ResourcePtr CreateLocal(TKClass* Class) = 0;

   public:
    std::unordered_map<String, ResourcePtr> m_storage;
    TKClass* m_baseType = nullptr;
  };

} // namespace ToolKit
