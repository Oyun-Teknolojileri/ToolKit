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

  struct TK_API TKClass
  {
    TKClass* Super = nullptr; //!< Compile time assigned base class for this class.
    String Name;              //!< Compile time assigned unique class name.

    bool operator==(const TKClass& other) const { return (Name == other.Name); }

    bool operator==(const TKClass* other) const { return (Name == other->Name); }

    bool operator!=(const TKClass& other) const { return this->Name != other.Name; }

    bool operator!=(const TKClass* other) const { return this->Name != other->Name; }

    /**
     * Checks if the class is of the same type of base ( equal or derived from base ).
     * @param base - The target class to check equality for.
     * @return true in case of this class being equal base or derived from base.
     */
    bool IsSublcassOf(TKClass* base);
  };

  typedef std::shared_ptr<class TKObject> TKObjectPtr;

#define TKDeclareClass(This, Base)                                                                                     \
 private:                                                                                                              \
  static TKClass This##Cls;                                                                                            \
  typedef Base Super;                                                                                                  \
                                                                                                                       \
 public:                                                                                                               \
  virtual TKClass* const Class() const;                                                                                \
  static TKClass* const StaticClass()                                                                                  \
  {                                                                                                                    \
    return &This##Cls;                                                                                                 \
  }

#define TKDefineClass(This, Base)                                                                                      \
  TKClass This::This##Cls = {Base::StaticClass(), #This};                                                              \
  TKClass* const This::Class() const                                                                                   \
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
    virtual TKObjectPtr Copy();

    template <class T>
    bool IsA()
    {
      return Class()->IsSublcassOf(T::StaticClass());
    }

    template <class T>
    T* As()
    {
      if (IsA<T>())
      {
        return static_cast<T*>(this);
      }

      return nullptr;
    }

   protected:
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerializeImp(XmlDocument* doc, XmlNode* parent) override;

   public:
    TKDeclareParam(ULongID, Id);

    ParameterBlock m_localData;
  };

} // namespace ToolKit