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

#include "ResourceManager.h"
#include "Serialize.h"
#include "Types.h"

namespace ToolKit
{

  extern TK_API ResourceManager* GetResourceManager(ResourceType type);

#define TKResourceType(type)                                                                                           \
  static ResourceType GetTypeStatic()                                                                                  \
  {                                                                                                                    \
    return ResourceType::type;                                                                                         \
  }                                                                                                                    \
  ResourceType GetType() const override                                                                                \
  {                                                                                                                    \
    return ResourceType::type;                                                                                         \
  }

  class TK_API Resource : public Serializable
  {
   public:
    Resource();
    virtual ~Resource();
    virtual void Load() = 0;
    virtual void Save(bool onlyIfDirty);
    virtual void Reload();

    virtual void Init(bool flushClientSideArray = false) = 0;
    virtual void UnInit()                                = 0;

    template <typename T>
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
    virtual XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const;
    virtual XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent);

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

    const String& GetFile() const;
    /**
     * Returns _missingFile if not empty to prevent override actual resource file.
     * Always call this if you are in Serialize function.
     * @returns A file path that preserves the original file path in case of a missing file.
     */
    const String& GetSerializeFile() const;

    /**
     * Sets the file for this resource.
     * @param file is path to resource file.
     */
    void SetFile(const String& file);

    /**
     * A resource is considered to be dynamic if it does not have a file.
     * This states that the object has been created on the runtime.
     * @returns Returns true if resource does not have a file path.
     */
    bool IsDynamic();

   protected:
    virtual void CopyTo(Resource* other);

    /**
     * Create SerializationFileInfo structure and pass it to DeSerializeImp.
     * @param firstNode is the name of root node of the xml file of this resource.
     */
    void ParseDocument(StringView firstNode);

   public:
    String m_name;
    ULongID m_id;
    bool m_dirty     = false;
    bool m_loaded    = false;
    bool m_initiated = false;

    /**
     * Internal usage.
     * When a file can't be located, a default resource gets assigned to the resource.
     * This member gets written with the original file. Purpose is to prevent data loss.
     * Because otherwise default resource' file gets written in place of original file.
     */
    String _missingFile;

   private:
    String m_file;
  };

} // namespace ToolKit
