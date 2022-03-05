#include "stdafx.h"
#include "Resource.h"
#include "Util.h"

namespace ToolKit
{
  ULongID Resource::m_handle = NULL_ENTITY;

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

    if (m_file.empty())
    {
      m_file = m_name + GetExtFromType(GetType());
      m_file = CreatePathFromResourceType(m_file, GetType());
    }

    std::ofstream file;
    file.open(m_file.c_str(), std::ios::out);
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
    assert(other->GetType() == GetType());
    if (!m_file.empty())
    {
      other->m_file = CreateCopyFileFullPath(m_file);
    }
    other->m_name = m_name;
    other->m_dirty = m_dirty;
    other->m_loaded = m_loaded;
    other->m_initiated = m_initiated;
  }

  ResourceType Resource::GetType() const
  {
    return ResourceType::Base;
  }

  void Resource::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    assert(false && "Not implemented");
  }

  void Resource::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    assert(false && "Not implemented");
  }

  String Resource::GetFile() const
  {
    return m_file;
  }

  const String& Resource::GetSerializeFile()
  {
    if (_missingFile.empty())
    {
      return m_file;
    }

    return _missingFile;
  }

  void Resource::SetFile(const String& file)
  {
    m_file = file;
  }

}