#include "stdafx.h"
#include "Resource.h"

namespace ToolKit
{

  Resource::~Resource()
  {
    // Nothing to do.
  }

  void Resource::Reload()
  {
    if (!m_file.empty())
    {
      UnInit();
      m_loaded = false;
      Load();
    }
  }

  Resource* Resource::GetCopy() 
  { 
    assert(false && "Not implemented"); 
    return nullptr; 
  }

}