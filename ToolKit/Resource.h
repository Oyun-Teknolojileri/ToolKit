#pragma once

#include <memory>

#include "Types.h"
#include "ResourceManager.h"
#include "Serialize.h"

namespace ToolKit
{

  extern TK_API ResourceManager* GetResourceManager(ResourceType type);

#define TKResouceType(type) \
  static ResourceType GetTypeStatic() { return ResourceType::type; } \
  ResourceType GetType() const override { return ResourceType::type; }

  class TK_API Resource : public Serializable
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
      std::shared_ptr<T> resource = std::make_shared<T>();
      CopyTo(resource.get());
      if (ResourceManager* manager = GetResourceManager(T::GetTypeStatic()))
      {
        manager->Manage(resource);
      }
      return resource;
    }
    virtual ResourceType GetType() const;
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent);
    void SerializeRef(XmlDocument* doc, XmlNode* parent);

    String GetFile() const;
    /**
    * Returns _missingFile if not empty to prevent override actual resource 
    * file. 
    * Always call this if you are in Serialize function.
    */
    const String& GetSerializeFile();
    void SetFile(const String& file);

   protected:
    virtual void CopyTo(Resource* other);

   public:
    String m_name;
    ULongID m_id;
    bool m_dirty = false;
    bool m_loaded = false;
    bool m_initiated = false;

    // Internal usage.
    String _missingFile;

   private:
    String m_file;
    static ULongID m_handle;
  };

}  // namespace ToolKit
