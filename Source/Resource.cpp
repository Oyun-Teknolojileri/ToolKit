#include "stdafx.h"
#include "Resource.h"

namespace ToolKit
{
  EntityId Resource::m_handle = NULL_ENTITY;

  Resource::Resource()
  {
    m_id = ++m_handle;
    m_name = "Resource_" + std::to_string(m_id);
  }

  Resource::~Resource()
  {
    // Nothing to do.
  }

  void Resource::Save(bool onlyIfDirty)
  {
    if (onlyIfDirty && !m_dirty)
    {
      return;
    }

    String fileName = m_file;
    if (fileName.empty())
    {
      fileName = m_name + MESH;
    }

    std::ofstream file;
    file.open(fileName.c_str(), std::ios::out);
    if (file.is_open())
    {
      XmlDocument doc;
      Serialize(&doc, nullptr);
      std::string xml;
      rapidxml::print(std::back_inserter(xml), doc, 0);

      file << xml;
      file.close();
      doc.clear();
    }
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

  void Resource::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    assert(false && "Not implemented");
  }

  void Resource::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    assert(false && "Not implemented");
  }

}