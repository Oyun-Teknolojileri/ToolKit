#pragma once

#include "Types.h"
#include "ResourceManager.h"

namespace ToolKit
{

  class Resource
  {
  public:
    virtual ~Resource();
    virtual void Load() = 0;
    virtual void Reload();

    virtual void Init(bool flushClientSideArray = true) = 0;
    virtual void UnInit() = 0;
    virtual Resource* GetCopy();

  public:
    String m_file;
    bool m_loaded = false;
    bool m_initiated = false;
    ResourceType m_type = ResourceType::Base;
  };

}