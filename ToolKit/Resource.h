/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Object.h"
#include "ObjectFactory.h"
#include "Types.h"

namespace ToolKit
{

  class TK_API Resource : public Object
  {
    friend class ResourceManager;

   public:
    TKDeclareClass(Resource, Object);

    Resource();
    virtual ~Resource();
    virtual void Load() = 0;
    virtual void Save(bool onlyIfDirty);
    virtual void Reload();

    virtual void Init(bool flushClientSideArray = false) = 0;
    virtual void UnInit()                                = 0;

    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

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
     * @param full - parse all the xml file along with comments.
     */
    void ParseDocument(StringView firstNode, bool fullParse = false);

   public:
    String m_name;
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

  typedef std::shared_ptr<Resource> ResourcePtr;
} // namespace ToolKit
