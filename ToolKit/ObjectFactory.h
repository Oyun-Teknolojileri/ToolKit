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

#include "ToolKit.h"
#include "Types.h"

#include <type_traits>

namespace ToolKit
{
  class Main;

  class TK_API TKObjectFactory
  {
    friend Main;
    typedef std::function<Object*()> ObjectConstructorCallback;

   public:
    /**
     * Helper function to identify if class T has a StaticClass function.
     */
    template <typename T>
    struct HasStaticClass
    {
      // Check if T has a StaticClass function
      template <typename U>
      static constexpr auto Check(U*) -> decltype(&U::StaticClass, std::true_type {});

      // Fallback for when StaticClass is not found
      template <typename>
      static constexpr std::false_type Check(...);

      // Combine the checks for T and its base classes up to Object
      static constexpr bool value = decltype(Check<T>(nullptr))::value;
    };

    typedef std::function<Object*()> ObjectConstructorCallback;        //!< Type for object constructor callbacks.
    typedef std::function<void(StringView val)> MetaProcessorCallback; //!< Type for MetaKey callbacks.

    /**
     * Registers or overrides the default constructor of given Object type.
     * @param constructorFn - This is the callback function that is responsible of creating the given object.
     */
    template <typename T>
    void Register(ObjectConstructorCallback constructorFn = []() -> T* { return new T(); })
    {
      TKClass* objectClass                  = T::StaticClass();
      m_constructorFnMap[objectClass->Name] = constructorFn;

      // TODO: Make the Id assignment collision free.
      T::StaticClass()->HashId              = std::hash<String> {}(objectClass->Name);

      // Iterate over all meta processors for each meta entry.
      for (auto& meta : objectClass->MetaKeys)
      {
        auto metaProcessor = m_metaProcessorMap.find(meta.first);
        if (metaProcessor != m_metaProcessorMap.end())
        {
          metaProcessor->second(meta.second);
        }
      }
    }

    /**
     * Alters the ObjectFactory such that when an object with BaseCls needed to be created, it creates the DerivedCls.
     * The purpose is the ability to create derived class of the engine such as EditorCamera. When you are loading a
     * scene within the editor, it should create EditorCamera instead of Camera. Also derived class takes the base class
     * name in order to get serialized as the base class. This also allows the base classes to be serialized instead of
     * the derived ones. So when a scene is serialized, instead of the EditorCamera, Camera will appear in the file.
     */
    template <typename DerivedCls, typename BaseCls>
    void Override(ObjectConstructorCallback constructorFn = []() -> DerivedCls* { return new DerivedCls(); })
    {
      DerivedCls::StaticClass()->Name = BaseCls::StaticClass()->Name;
      Register<DerivedCls>(constructorFn);
      Register<BaseCls>(constructorFn);
    }

    /**
     * Each MetaKey has a corresponding meta processor. When a class registered and it has a MetaKey that corresponds to
     * one of MetaProcessor in the map, processor gets called with MetaKey's value.
     */
    std::unordered_map<StringView, MetaProcessorCallback> m_metaProcessorMap;

    /**
     * Returns the Constructor for given class name.
     */
    ObjectConstructorCallback& GetConstructorFn(const StringView Class);

    /**
     * Constructs a new Object from class name.
     * @param Class is the class name of the object to be created.
     * @return A new instance of the object with the given class name.
     */
    Object* MakeNew(const StringView Class);

    /**
     * Constructs a new Object of type T. In case the T does not have a static class, just returns a regular object.
     * @return A new instance of Object.
     */
    template <typename T, typename... Args>
    T* MakeNew(Args&&... args)
    {
      if constexpr (HasStaticClass<T>::value)
      {
        if (auto constructorFn = GetConstructorFn(T::StaticClass()->Name))
        {
          Object* object  = constructorFn();
          T* castedObject = static_cast<T*>(object);
          castedObject->NativeConstruct(std::forward<Args>(args)...);

          return castedObject;
        }
        else
        {
          assert(false && "Unknown object type.");
          return nullptr;
        }
      }
      else
      {
        return new T(std::forward<Args>(args)...);
      }

      return nullptr;
    }

   private:
    TKObjectFactory();
    ~TKObjectFactory();
    TKObjectFactory(const TKObjectFactory&)            = delete;
    TKObjectFactory(TKObjectFactory&&)                 = delete;
    TKObjectFactory& operator=(const TKObjectFactory&) = delete;
    TKObjectFactory& operator=(TKObjectFactory&&)      = delete;

    /**
     * Registers all the known Object constructors.
     */
    void Init();

   private:
    std::unordered_map<StringView, ObjectConstructorCallback> m_constructorFnMap;
    ObjectConstructorCallback m_nullFn = nullptr;
  };

  template <typename T, typename... Args>
  T* MakeNew(Args&&... args)
  {
    if (Main* main = Main::GetInstance())
    {
      if (TKObjectFactory* of = main->m_objectFactory)
      {
        return of->MakeNew<T>(std::forward<Args>(args)...);
      }
    }

    return nullptr;
  }

  template <typename T, typename... Args>
  std::shared_ptr<T> MakeNewPtr(Args&&... args)
  {
    if (Main* main = Main::GetInstance())
    {
      if (TKObjectFactory* of = main->m_objectFactory)
      {
        return std::shared_ptr<T>(of->MakeNew<T>(std::forward<Args>(args)...));
      }
    }

    return nullptr;
  }

  template <typename T, typename... Args>
  std::shared_ptr<T> MakeNewPtrCasted(const StringView Class, Args&&... args)
  {
    if (Main* main = Main::GetInstance())
    {
      if (TKObjectFactory* of = main->m_objectFactory)
      {
        return std::shared_ptr<T>(static_cast<T*>(of->MakeNew(Class, std::forward<Args>(args)...)));
      }
    }

    return nullptr;
  }

  template <typename T>
  std::shared_ptr<T> Cast(TKObjectPtr tkObj)
  {
    return std::static_pointer_cast<T>(tkObj);
  }

} // namespace ToolKit