#include "Resource.h"

#include <string>
#include "Util.h"
#include "ToolKit.h"

namespace ToolKit
{

  Resource::Resource()
  {
    m_id = GetHandleManager()->GetNextHandle();
    m_name = "Resource_" + std::to_string(m_id);
  }

  Resource::~Resource()
  {
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

  bool Resource::IsDynamic()
  {
    return GetFile().empty();
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

  void Resource::SerializeRef(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* refNode = CreateXmlNode(doc, XmlResRefElement, parent);
    WriteAttr
    (
      refNode, doc, "Type",
      std::to_string
      (
        static_cast<int> (GetType())
      )
    );

    String file = GetSerializeFile();
    file = GetRelativeResourcePath(file);
    WriteAttr(refNode, doc, "File", file);
  }

  String Resource::DeserializeRef(XmlNode* parent)
  {
    String val;
    if (XmlNode* refNode = parent->first_node(XmlResRefElement.c_str()))
    {
      ReadAttr(refNode, "File", val);
      NormalizePath(val);
    }
    return val;
  }

  String Resource::GetFile() const
  {
    return m_file;
  }

  const String& Resource::GetSerializeFile() const
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

}  // namespace ToolKit
