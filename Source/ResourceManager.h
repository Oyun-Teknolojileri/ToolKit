#pragma once

#include "Util.h"
#include "Logger.h"

#include <memory>
#include <unordered_map>
#include <string>

namespace ToolKit
{

  template <class T>
  class ResourceManager
  {
  public:

    virtual void Init()
    {
      Logger::GetInstance()->Log("Initiating manager " + String(typeid(T).name()));
    }

    virtual void Uninit()
    {
      Logger::GetInstance()->Log("Uninitiating manager " + String(typeid(T).name()));
      m_storage.clear();
    }

    virtual ~ResourceManager()
    {
      assert(m_storage.size() == 0); // Uninitialize all resources before exit.
    }

    template<typename Ti = T>
    std::shared_ptr<T> Create(String file)
    {
      if (!Exist(file))
      {
        bool fileCheck = CheckFile(file);
        if (!fileCheck)
        {
          Logger::GetInstance()->Log("Missing: " + file);
          assert(fileCheck);
        }

        T* resource = new Ti(file);
        resource->Load();
        m_storage[file] = std::shared_ptr<T>(resource);
      }

      return m_storage[file];
    }

    template<typename Ti>
    std::shared_ptr<Ti> CreateDerived(String file)
    {
      return std::static_pointer_cast<Ti> (Create<Ti>(file));
    }

    bool Exist(String file)
    {
      return m_storage.find(file) != m_storage.end();
    }

  public:
    std::unordered_map<String, std::shared_ptr<T>> m_storage;
  };

}