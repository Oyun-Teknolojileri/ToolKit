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

    /** Returns loaded percent of the serializable object. */
    float GetLoadedPercent() { return 100.0f * m_numberOfThingsLoaded / m_numberOfThingsToLoad; }

    /** Returns if loading of the serializable object is completed. */
    bool IsLoadingComplated() { return glm::greaterThanEqual(GetLoadedPercent(), 100.0f); }

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

    virtual void PreDeserializeImp(const SerializationFileInfo& info, XmlNode* parent) { m_numberOfThingsLoaded = 0; }

    virtual XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) = 0;

    virtual void PostDeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
    {
      // At this point all loading must be complete.
      m_numberOfThingsLoaded = m_numberOfThingsToLoad;
    }

   public:
    String m_version = TKVersionStr;

   protected:
    /** Total number of things that will be loading. Used in completed percent calculation. */
    int m_numberOfThingsToLoad = 1;

    /** Total number of things got loaded. Used in completed percent calculation. */
    int m_numberOfThingsLoaded = 0;
  };

} // namespace ToolKit
