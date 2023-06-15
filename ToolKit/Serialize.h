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

  class TK_API Serializable
  {
   public:
    void Serialize(XmlDocument* doc, XmlNode* parent) const
    {
      PreSerializeImp(doc, parent);
      SerializeImp(doc, parent);
      PostSerializeImp(doc, parent);
    }

    void DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      PreDeserializeImp(doc, parent);
      DeSerializeImp(doc, parent);
      PostDeSerializeImp(doc, parent);
    }

   protected:
    virtual void PreSerializeImp(XmlDocument* doc, XmlNode* parent) const {}

    virtual void SerializeImp(XmlDocument* doc, XmlNode* parent) const = 0;

    virtual void PostSerializeImp(XmlDocument* doc, XmlNode* parent) const {}

    virtual void PreDeserializeImp(XmlDocument* doc, XmlNode* parent) {}

    virtual void DeSerializeImp(XmlDocument* doc, XmlNode* parent) = 0;

    virtual void PostDeSerializeImp(XmlDocument* doc, XmlNode* parent) {}
  };

} // namespace ToolKit
