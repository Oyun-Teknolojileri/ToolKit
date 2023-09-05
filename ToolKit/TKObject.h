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
    TKClass* Super = nullptr;     //!< Compile time assigned base class for this class.
    String Name;                  //!< Compile time assigned unique class name.
    ULongID HashId = NULL_HANDLE; //!< Unique has id assigned to class when registered to TKObjectFactory.

    bool operator==(const TKClass& other) const
    {
      assert(HashId != NULL_HANDLE && "Class is not registered.");
      return (HashId == other.HashId);
    }

    bool operator==(const TKClass* other) const
    {
      assert(HashId != NULL_HANDLE && "Class is not registered.");
      return (HashId == other->HashId);
    }

    bool operator!=(const TKClass& other) const
    {
      assert(HashId != NULL_HANDLE && "Class is not registered.");
      return HashId != other.HashId;
    }

    bool operator!=(const TKClass* other) const
    {
      assert(HashId != NULL_HANDLE && "Class is not registered.");
      return HashId != other->HashId;
    }

    /**
     * Checks if the class is of the same type of base ( equal or derived from base ).
     * @param base - The target class to check equality for.
     * @return true in case of this class being equal base or derived from base.
     */
    bool IsSublcassOf(TKClass* base);
  };

  typedef std::shared_ptr<class TKObject> TKObjectPtr;

#define TKDeclareClassBase(This, Base)                                                                                 \
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

#define TKDeclareClass(This, Base) TKDeclareClassBase(This, Base) using TKObject::NativeConstruct;

#define TKDefineClass(This, Base)                                                                                      \
  TKClass This::This##Cls = {Base::StaticClass(), #This};                                                              \
  TKClass* const This::Class() const                                                                                   \
  {                                                                                                                    \
    return &This##Cls;                                                                                                 \
  }

  class TK_API TKObject : public Serializable
  {
    TKDeclareClassBase(TKObject, TKObject);

   public:
    TKObject();
    virtual ~TKObject();
    virtual void NativeConstruct();
    virtual void NativeDestruct();
    virtual TKObjectPtr Copy() const;

    template <typename T>
    bool IsA()
    {
      return Class()->IsSublcassOf(T::StaticClass());
    }

    template <typename T>
    T* As()
    {
      if (IsA<T>())
      {
        return static_cast<T*>(this);
      }

      return nullptr;
    }

   protected:
    virtual void ParameterConstructor();
    virtual void ParameterEventConstructor();
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    void PostDeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

   public:
    TKDeclareParam(ULongID, Id);

    ParameterBlock m_localData;
  };

} // namespace ToolKit