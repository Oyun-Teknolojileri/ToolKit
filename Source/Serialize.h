#pragma once

#include "Types.h"

namespace ToolKit
{
  
  class Serializable {
  public:
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const = 0;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) = 0;
  };

}
