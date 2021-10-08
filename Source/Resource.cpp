#include "stdafx.h"
#include "Resource.h"
#include "Util.h"

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
      fileName = m_name + GetExtFromType(m_type);
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

  void Resource::CopyTo(Resource* other) 
  { 
    assert(other->m_type == m_type);
    if (!m_file.empty())
    {
      m_file = CreateCopyFileFullPath(m_file);
    }
    other->m_name = m_name;
    other->m_dirty = m_dirty;
    other->m_loaded = m_loaded;
    other->m_initiated = m_initiated;
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