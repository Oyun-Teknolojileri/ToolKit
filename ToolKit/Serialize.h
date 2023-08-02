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
