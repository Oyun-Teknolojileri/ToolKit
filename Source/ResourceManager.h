#pragma once

#include "Types.h"
#include "Util.h"
#include "Logger.h"

#include <unordered_map>
#include <memory>

namespace ToolKit
{

  extern class Logger* GetLogger();

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
    Scene
  };

  class ResourceManager
  {
  public:
    virtual void Init();
    virtual void Uninit();
    virtual void Manage(const ResourcePtr& resource);
    virtual ~ResourceManager();

    template<typename T>
    std::shared_ptr<T> Create(String file)
    {
      if (!Exist(file))
      {
        bool fileCheck = CheckFile(file);
        if (!fileCheck)
        {
          GetLogger()->Log("Missing: " + file);
          assert(fileCheck);
          return nullptr;
        }

        std::shared_ptr<T> resource = std::make_shared<T>(file);

        resource->Load();
        m_storage[file] = resource;
      }

      return std::reinterpret_pointer_cast<T> (m_storage[file]);
    }

    bool Exist(String file);

  public:
    std::unordered_map<String, ResourcePtr> m_storage;
    ResourceType m_type = ResourceType::Base;
  };

}