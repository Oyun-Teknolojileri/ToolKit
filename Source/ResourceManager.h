#pragma once

#include "Types.h"
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
    RenderTarget
  };

  class ResourceManager
  {
  public:
    virtual void Init();
    virtual void Uninit();
    virtual ~ResourceManager();

    virtual ResourcePtr Create(const String& file);
    virtual ResourcePtr Create(const String& file, ResourceType type);

    template<typename T>
    std::shared_ptr<T> Create(const String& file)
    {
      return std::static_pointer_cast<T> (Create(file, m_type));
    }

    template<typename T>
    std::shared_ptr<T> Create(const String& file, ResourceType type)
    {
      return std::static_pointer_cast<T> (Create(file, type));
    }

    bool Exist(String file);

  public:
    std::unordered_map<String, ResourcePtr> m_storage;
    ResourceType m_type = ResourceType::Base;
  };

}