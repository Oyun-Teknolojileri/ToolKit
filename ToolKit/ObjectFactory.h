/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Logger.h"
#include "ToolKit.h"
#include "Types.h"

#include <type_traits>

namespace ToolKit
{

  class TK_API ObjectFactory
  {
    friend class Main;

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
    void Register(
        ObjectConstructorCallback constructorFn = []() -> T* { return new T(); },
        bool overrideClass                      = false)
    {
      ClassMeta* objectClass = T::StaticClass();

      if (!overrideClass)
      {
        // Sanity check
        auto classItr = m_allRegisteredClasses.find(objectClass->HashId);
        if (classItr != m_allRegisteredClasses.end())
        {
          String& clsName = classItr->second->Name;
          if (clsName == objectClass->Name)
          {
            TK_ERR("Registering the same class multiple times: %s", clsName.c_str());
            assert(false && "Registering the same class multiple times");
          }
          else
          {
            ToolKit::GetLogger()->Log(LogType::Error,
                                      "Hash collision between Class: %s and Class: %s",
                                      objectClass->Name.c_str(),
                                      clsName.c_str());

            assert(false && "Hash collision.");
            std::exit(-1);
          }
        }
      }

      m_allRegisteredClasses.insert({objectClass->HashId, objectClass});

      m_constructorFnMap[objectClass->Name] = constructorFn;

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

    template <typename T>
    void Unregister()
    {
      m_constructorFnMap.erase(T::StaticClass()->Name);
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
      Register<DerivedCls>(constructorFn, true);
      Register<BaseCls>(constructorFn, true);
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
    template <typename T>
    T* MakeNew()
    {
      if constexpr (HasStaticClass<T>::value)
      {
        if (auto constructorFn = GetConstructorFn(T::StaticClass()->Name))
        {
          Object* object  = constructorFn();
          T* castedObject = static_cast<T*>(object);
          return castedObject;
        }
      }

      assert(false && "Unknown object type.");
      return nullptr;
    }

   private:
    ObjectFactory();
    ~ObjectFactory();
    ObjectFactory(const ObjectFactory&)            = delete;
    ObjectFactory(ObjectFactory&&)                 = delete;
    ObjectFactory& operator=(const ObjectFactory&) = delete;
    ObjectFactory& operator=(ObjectFactory&&)      = delete;

    /**
     * Registers all the known Object constructors.
     */
    void Init();

   private:
    std::unordered_map<StringView, ObjectConstructorCallback> m_constructorFnMap;
    ObjectConstructorCallback m_nullFn = nullptr;
    std::unordered_map<ULongID, ClassMeta*> m_allRegisteredClasses;
  };

  template <typename T, typename... Args>
  inline std::shared_ptr<T> MakeNewPtr(Args&&... args)
  {
    if (Main* main = Main::GetInstance())
    {
      if (ObjectFactory* of = main->m_objectFactory)
      {
        if constexpr (ObjectFactory::HasStaticClass<T>::value)
        {
          std::shared_ptr<T> obj = std::shared_ptr<T>(of->MakeNew<T>());
          obj->m_self            = obj;
          obj->NativeConstruct(std::forward<Args>(args)...);
          return obj;
        }
        else
        {
          return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
        }
      }
    }

    return nullptr;
  }

  template <typename T, typename... Args>
  inline std::shared_ptr<T> MakeNewPtrCasted(const StringView Class, Args&&... args)
  {
    if (Main* main = Main::GetInstance())
    {
      if (ObjectFactory* of = main->m_objectFactory)
      {
        std::shared_ptr<T> obj = std::shared_ptr<T>(static_cast<T*>(of->MakeNew(Class)));

        if constexpr (ObjectFactory::HasStaticClass<T>::value)
        {
          obj->m_self = obj;
          obj->NativeConstruct(std::forward<Args>(args)...);
        }

        return obj;
      }
    }

    return nullptr;
  }

  template <typename T>
  inline std::shared_ptr<T> Cast(ObjectPtr tkObj)
  {
    return std::static_pointer_cast<T>(tkObj);
  }

} // namespace ToolKit