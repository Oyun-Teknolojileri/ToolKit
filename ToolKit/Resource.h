#pragma once

#include <memory>

#include "Types.h"
#include "ResourceManager.h"
#include "Serialize.h"

namespace ToolKit
{

  extern TK_API ResourceManager* GetResourceManager(ResourceType type);

#define TKResourceType(type) \
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

    /**
    * Outputs file path and the resource type to an xml node. Xml node name is
    * ResourceRef and xml node has Type attribute for resource type enum and
    * File for resource path.
    * @param doc Xml document to append the reference node.
    * @param parent Xml Node to append the reference.
    */
    void SerializeRef(XmlDocument* doc, XmlNode* parent) const;

    /**
    * Extracts the File attribute from the ResourceRef Xml Node.
    * @param parent Parent xml node that contains reference node.
    * @returns File path to the resource to be referenced.
    */
    static String DeserializeRef(XmlNode* parent);

    String GetFile() const;
    /**
    * Returns _missingFile if not empty to prevent override actual resource 
    * file. 
    * Always call this if you are in Serialize function.
    */
    const String& GetSerializeFile() const;
    void SetFile(const String& file);

    /**
    * A resource is considered to be dynamic if it does not have a file.
    * This states that the object has been created on the runtime.
    * @returns Returns true if resource does not have a file path.
    */
    bool IsDynamic();

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
  };

}  // namespace ToolKit
