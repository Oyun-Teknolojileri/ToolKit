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

#include "ParameterBlock.h"
#include "Serialize.h"
#include "Types.h"

namespace ToolKit
{

  struct TKClass
  {
    TKClass* Super = nullptr;
    String Name;

    bool IsSublcassOf(TKClass* base)
    {
      if (Super == nullptr)
      {
        return false;
      }

      if (base->Name == Super->Name)
      {
        return true;
      }

      return IsSublcassOf(Super);
    }
  };

  typedef std::shared_ptr<class TKObject> TKObjectPtr;

#define TKDeclareClass(This, Base)                                                                                     \
 private:                                                                                                              \
  static TKClass This##Cls;                                                                                            \
  typedef Base Super;                                                                                                  \
                                                                                                                       \
 public:                                                                                                               \
  virtual TKClass* const Class();                                                                                      \
  static TKClass* const StaticClass()                                                                                  \
  {                                                                                                                    \
    return &This##Cls;                                                                                                 \
  }

#define TKDefineClass(This, Base)                                                                                      \
  TKClass This::This##Cls = {Base::StaticClass(), #This};                                                              \
  TKClass* const This::Class()                                                                                         \
  {                                                                                                                    \
    return &This##Cls;                                                                                                 \
  }

  class TK_API TKObject : public Serializable
  {
    TKDeclareClass(TKObject, TKObject);

   public:
    TKObject();
    virtual ~TKObject();
    virtual void NativeConstruct();
    virtual void NativeDestruct();
    virtual void ParameterConstructor();
    virtual void ParameterEventConstructor();
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent);
    virtual TKObjectPtr Copy();

   public:
    TKDeclareParam(ULongID, Id);

   private:
    ParameterBlock m_localData;
  };

} // namespace ToolKit