#pragma once

#include <unordered_map>
#include <memory>

#include "Types.h"
#include "Util.h"
#include "Logger.h"

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
    Skeleton
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

    ResourceManager(ResourceManager const&) = delete;
    void operator=(ResourceManager const&) = delete;

    template<typename T>
    std::shared_ptr<T> Create(const String& file)
    {
      if (!Exist(file))
      {
        std::shared_ptr<T> resource = std::static_pointer_cast<T>
        (
          CreateLocal(T::GetTypeStatic())
        );
        if (!CheckFile(file))
        {
          String def = GetDefaultResource(T::GetTypeStatic());
          if (!CheckFile(def))
          {
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

      return std::reinterpret_pointer_cast<T> (m_storage[file]);
    }

    bool Exist(String file);
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

}  // namespace ToolKit
