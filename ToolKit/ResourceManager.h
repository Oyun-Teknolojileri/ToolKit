/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
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

  TK_API extern class ResourceManager* GetResourceManager(ClassMeta* Class);

  class TK_API ResourceManager
  {
   public:
    ResourceManager();
    virtual ~ResourceManager();
    virtual void Init();
    virtual void Uninit();
    virtual void Manage(ResourcePtr resource);
    virtual bool CanStore(ClassMeta* Class) = 0;
    virtual String GetDefaultResource(ClassMeta* Class);

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
            TK_ERR("No default for Class %s", T::StaticClass()->Name.c_str());
            assert(0 && "No default resource!");
            return nullptr;
          }

          String rel = GetRelativeResourcePath(file);
          TK_WRN("File: %s is missing. Using default resource.", rel.c_str());
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
    std::shared_ptr<T> Copy(ResourcePtr source, bool storeInResourceManager = true)
    {
      std::shared_ptr<T> resource = MakeNewPtr<T>();
      source->CopyTo(resource.get());
      ResourceManager* manager = GetResourceManager(T::StaticClass());
      if (manager != nullptr && storeInResourceManager)
      {
        manager->Manage(resource);
      }
      return resource;
    }

    bool Exist(const String& file);
    ResourcePtr Remove(const String& file);

   public:
    std::unordered_map<String, ResourcePtr> m_storage;
    ClassMeta* m_baseType = nullptr;
  };

} // namespace ToolKit
