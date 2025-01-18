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

  /** Progress callback for loading. */
  typedef std::function<void(float)> ProgressCallback;

  /** Serializaiton info for loading. */
  struct TK_API SerializationFileInfo
  {
    String File;
    String Version;
    XmlDocument* Document = nullptr;
  };

  /** Serializable object base class. */
  class TK_API Serializable
  {
   public:
    /** Serialize the object to given document from the given parent. */
    void Serialize(XmlDocument* doc, XmlNode* parent) const
    {
      PreSerializeImp(doc, parent);
      SerializeImp(doc, parent);
      PostSerializeImp(doc, parent);
    }

    /** Deserialize the object from given ducment from the given parent. */
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

    /** Sets progress callback function, which called with the complated percent every time progress updated. */
    void SetProgressCallback(ProgressCallback callback) { m_progressCallback = callback; }

    /**
     * Used to update loading progress.
     * Updates the m_numberOfThingsLoaded with given loadedCount number and calls the progress callbacks.
     */
    void UpdateProgress(uint loadedCount)
    {
      m_numberOfThingsLoaded += loadedCount;
      if (m_progressCallback)
      {
        m_progressCallback(GetLoadedPercent());
      }
    }

   protected:
    /** Pre serialization function. */
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

    /** Post serialization function. */
    virtual void PostSerializeImp(XmlDocument* doc, XmlNode* parent) const {}

    /** Pre deserialization function. */
    virtual void PreDeserializeImp(const SerializationFileInfo& info, XmlNode* parent) { m_numberOfThingsLoaded = 0; }

    /** Deserialization implementation function. Every serializable function must define it. */
    virtual XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) = 0;

    /** Post deserialization function. */
    virtual void PostDeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
    {
      // At this point all loading must be complete.
      m_numberOfThingsLoaded = m_numberOfThingsToLoad;
    }

   public:
    /** Version of the serializable object. */
    String m_version = TKVersionStr;

   protected:
    /** Total number of things that will be loading. Used in completed percent calculation. */
    uint m_numberOfThingsToLoad = 1;

   private:
    /** Total number of things got loaded. Used in completed percent calculation. */
    uint m_numberOfThingsLoaded         = 0;

    /** Progress callback for loading. */
    ProgressCallback m_progressCallback = nullptr;
  };

} // namespace ToolKit
