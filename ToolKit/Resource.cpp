/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Resource.h"

#include "FileManager.h"
#include "Material.h"
#include "Mesh.h"
#include "ResourceManager.h"
#include "Skeleton.h"
#include "ToolKit.h"
#include "Util.h"

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(Resource, Object);

  Resource::Resource()
  {
    static ULongID globalCounter = 1;
    m_name                       = "Resource_" + std::to_string(globalCounter++);
  }

  Resource::~Resource() {}

  void Resource::Save(bool onlyIfDirty)
  {
    if (onlyIfDirty && !m_dirty)
    {
      return;
    }

    if (m_file.empty())
    {
      m_file = m_name + GetExtFromType(Class());
      m_file = CreatePathFromResourceType(m_file, Class());
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

      m_dirty = false;
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

  bool Resource::IsDynamic() { return GetFile().empty(); }

  void Resource::CopyTo(Resource* other)
  {
    assert(other->Class() == Class());
    if (!m_file.empty())
    {
      other->m_file = CreateCopyFileFullPath(m_file);
    }
    other->m_name      = m_name;
    other->m_dirty     = m_dirty;
    other->m_loaded    = m_loaded;
    other->m_initiated = m_initiated;
  }

  void Resource::ParseDocument(StringView firstNode, bool fullParse)
  {
    SerializationFileInfo info;
    info.File          = GetFile();

    XmlFilePtr file    = GetFileManager()->GetXmlFile(info.File);
    XmlDocumentPtr doc = MakeNewPtr<XmlDocument>();

    if (fullParse)
    {
      doc->parse<rapidxml::parse_full>(file->data());
    }
    else
    {
      doc->parse<rapidxml::parse_default>(file->data());
    }

    info.Document = doc.get();
    if (XmlNode* rootNode = doc->first_node(firstNode.data()))
    {
      ReadAttr(rootNode, XmlVersion.data(), info.Version, TKV044);
      m_version = info.Version;

      DeSerialize(info, rootNode);
    }
  }

  XmlNode* Resource::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    assert(false && "Not implemented");
    return nullptr;
  }

  XmlNode* Resource::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    assert(false && "Not implemented");
    return nullptr;
  }

  void Resource::SerializeRef(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* refNode = CreateXmlNode(doc, XmlResRefElement, parent);
    WriteAttr(refNode, doc, "Class", Class()->Name);

    String file = GetSerializeFile();
    file        = GetRelativeResourcePath(file);
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

  const String& Resource::GetFile() const { return m_file; }

  const String& Resource::GetSerializeFile() const
  {
    if (_missingFile.empty())
    {
      return m_file;
    }

    return _missingFile;
  }

  void Resource::SetFile(const String& file) { m_file = file; }

} // namespace ToolKit
