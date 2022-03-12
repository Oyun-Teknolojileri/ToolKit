#pragma once

#include "Types.h"
#include "Serialize.h"

namespace ToolKit
{

  enum class ComponentType
  {
    Component_Base
  };

  class Component : public Serializable
  {
  public:
    Component();
    virtual  ~Component();

  public:
    ULongID m_id;
    ComponentType m_type;

  private:
    static ULongID m_handle;
  };

}
