/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

namespace ToolKit
{

  struct TK_API SerializationFileInfo
  {
    String File;
    String Version;
    XmlDocument* Document = nullptr;
  };

  class TK_API Serializable
  {
   public:
    void Serialize(XmlDocument* doc, XmlNode* parent) const
    {
      PreSerializeImp(doc, parent);
      SerializeImp(doc, parent);
      PostSerializeImp(doc, parent);
    }

    void DeSerialize(const SerializationFileInfo& info, XmlNode* parent)
    {
      PreDeserializeImp(info, parent);
      DeSerializeImp(info, parent);
      PostDeSerializeImp(info, parent);
    }

   protected:
    virtual void PreSerializeImp(XmlDocument* doc, XmlNode* parent) const {}

    /**
     * The implementation of the serialization function.
     * @param doc - This is the xml document this object will be serialized to.
     * @param parent - This is the Xml Element that this object will be serialized in.
     * @return - XmlNode* which this object's root node will written to. If an object is recursively serialized,
     * return node can be used as derived class's parent. Which provides a structure printed to xml from parent to
     * child.
     */
    virtual XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const = 0;

    virtual void PostSerializeImp(XmlDocument* doc, XmlNode* parent) const {}

    virtual void PreDeserializeImp(const SerializationFileInfo& info, XmlNode* parent) {}

    virtual XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) = 0;

    virtual void PostDeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) {}

   public:
    String m_version = TKVersionStr;
  };

} // namespace ToolKit
