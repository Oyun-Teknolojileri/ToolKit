#pragma once

#include "Types.h"
#include "Util.h"
#include "Logger.h"

#include <unordered_map>
#include <memory>

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

    ResourceManager(ResourceManager const&) = delete;
    void operator=(ResourceManager const&) = delete;

    template<typename T>
    std::shared_ptr<T> Create(const String& file)
    {
      if (!Exist(file))
      {
        if (!IsSane(file))
        {
          return nullptr;
        }

        std::shared_ptr<T> resource = std::static_pointer_cast<T> (CreateLocal(T::GetTypeStatic()));
        resource->m_file = file;
        resource->Load();
        m_storage[file] = resource;
      }

      return std::reinterpret_pointer_cast<T> (m_storage[file]);
    }

    bool Exist(String file);
    ResourcePtr Remove(const String& file);
    virtual ResourcePtr CreateLocal(ResourceType type) = 0;

  private:
    bool IsSane(const String& file);

  public:
    std::unordered_map<String, ResourcePtr> m_storage;
    ResourceType m_type = ResourceType::Base;
  };

}