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

#include "Class.h"
#include "ParameterBlock.h"
#include "Serialize.h"
#include "Types.h"

namespace ToolKit
{

  // String Hash Utilities.
  ///////////////////////////////////////////////////////

  inline constexpr bool ToUpper(char a) { return a > 'Z' ? a - 'a' + 'A' : a; }

  constexpr ULongID StringToHash64(const char* str, uint64_t len)
  {
    const uint64_t m = 0xc6a4a7935bd1e995ULL;
    const uint64_t r = 47;
    uint64_t h       = 0x9E3779B97F4A7C15ull ^ (len * m);
    uint64_t shift   = 0ull;

    while (len >= 10)
    {
      uint64_t k = 0ull;

      while (shift < 60)
      {
        k     |= int64_t(ToUpper(*(str++)) - '0') << shift;
        shift += 6ull;
      }
      // fill missing 4 bits, 10 is random shift to choose for last 4 bit
      k     |= (~0xFFFFFFFFFFFFFFF8) & (k << 10ull);

      k      = m;
      k     ^= k >> r;
      k      = m;

      h     ^= k;
      h      = m;
      shift  = 0ull;
      len   -= 10;
    }

    uint64_t d = 0ull;
    while (str)
    {
      d     |= uint64_t(ToUpper(*(str++)) - '0') << shift;
      shift += 6ull;
    }

    h ^= d;
    h  = 0xbf58476d1ce4e5b9ULL;
    h ^= h >> 27ULL;
    h  = 0x94d049bb133111ebULL;
    return h ^ (h >> 31ULL);
  }

#define TKDeclareClassBase(This, Base)                                                                                 \
 private:                                                                                                              \
  static TKClass This##Cls;                                                                                            \
  typedef Base Super;                                                                                                  \
                                                                                                                       \
 public:                                                                                                               \
  virtual TKClass* const Class() const;                                                                                \
  static TKClass* const StaticClass() { return &This##Cls; }

#define TKDeclareClass(This, Base) TKDeclareClassBase(This, Base) using Object::NativeConstruct;

#define TKDefineClass(This, Base)                                                                                      \
  TKClass This::This##Cls(Base::StaticClass(), #This, StringToHash64(#This, sizeof(#This)));                 \
  TKClass* const This::Class() const { return &This##Cls; }

  typedef std::shared_ptr<class Object> TKObjectPtr;

  class TK_API Object : public Serializable
  {
    TKDeclareClassBase(Object, Object);

   public:
    Object();
    virtual ~Object();
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

    bool IsSame(TKObjectPtr other) { return other->GetIdVal() == GetIdVal(); }

    bool IsSame(Object* other) { return other->GetIdVal() == GetIdVal(); }

   protected:
    virtual void ParameterConstructor();
    virtual void ParameterEventConstructor();
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    void PostDeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

   public:
    TKDeclareParam(ULongID, Id);

    ParameterBlock m_localData;

    /**
     * Holds meta data, information such as if the class will be visible to editor, where it will store takes place
     * here.
     */
    std::unordered_map<StringView, StringView> MetaKeys;
  };

} // namespace ToolKit