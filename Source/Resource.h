#pragma once

#include "Types.h"
#include "ResourceManager.h"
#include "Serialize.h"

namespace ToolKit
{

  class Resource : public Serializable
  {
  public:
    Resource();
    virtual ~Resource();
    virtual void Load() = 0;
    virtual void Save(bool onlyIfDirty);
    virtual void Reload();

    virtual void Init(bool flushClientSideArray = true) = 0;
    virtual void UnInit() = 0;
    
    template<typename T>
    std::shared_ptr<T> Copy()
    {
      T* cpy = new T();
      CopyTo(cpy);
      std::shared_ptr<T> resource = std::shared_ptr<T> (cpy);
      if (ResourceManager* manager = GetResourceManager(resource->m_type))
      {
        manager->Manage(resource);
      }
      return resource;
    }

    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent);

  protected:
    virtual void CopyTo(Resource* other);

  public:
    String m_file;
    String m_name;
    EntityId m_id;
    bool m_dirty = false;
    bool m_loaded = false;
    bool m_initiated = false;
    ResourceType m_type = ResourceType::Base;

  private:
    static EntityId m_handle;
  };

}