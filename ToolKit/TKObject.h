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
    virtual TKObjectPtr Copy();

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

   public:
    TKDeclareParam(ULongID, Id);

    ParameterBlock m_localData;
  };

  typedef std::function<TKObject*()> ObjectConstructorCallback;

  class TK_API TKObjectFactory
  {
    friend class Main;

    TKObjectFactory();
    ~TKObjectFactory();
    TKObjectFactory(const TKObjectFactory&)            = delete;
    TKObjectFactory(TKObjectFactory&&)                 = delete;
    TKObjectFactory& operator=(const TKObjectFactory&) = delete;
    TKObjectFactory& operator=(TKObjectFactory&&)      = delete;

   public:
    /**
     * Registers or overrides the default constructor of given TKObject type.
     * @param constructorFn - This is the callback function that is responsible of creating the given object.
     */
    template <typename T>
    void Register(ObjectConstructorCallback constructorFn = []() -> T* { return new T(); })
    {
      m_constructorFnMap[T::StaticClass()->Name] = constructorFn;
    }

    /**
     * Constructs a new TKObject from class name.
     * @param cls - Class name of the object to be created.
     * @return A new instance of the object with the given class name.
     */
    TKObject* MakeNew(const StringView& cls);

    /**
     * Constructs a new TKObject of type T.
     * @return A new instance of TKObject.
     */
    template <typename T>
    T* MakeNew()
    {
      if (TKObject* object = MakeNew(T::StaticClass()->Name))
      {
        return static_cast<T*>(object);
      }

      return nullptr;
    }

   private:
    /**
     * Registers all the known TKObject constructors.
     */
    void Init();

   private:
    std::unordered_map<StringView, ObjectConstructorCallback> m_constructorFnMap;
  };

} // namespace ToolKit