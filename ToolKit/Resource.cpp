/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Resource.h"

#include "FileManager.h"
#include "ToolKit.h"
#include "Util.h"

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(Resource, TKObject);

  Resource::Resource()
  {
    m_id   = GetHandleManager()->GetNextHandle();
    m_name = "Resource_" + std::to_string(m_id);
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
    assert(other->GetType() == GetType());
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
    info.File          = GetSerializeFile();

    XmlFilePtr file    = GetFileManager()->GetXmlFile(info.File);
    XmlDocumentPtr doc = std::make_shared<XmlDocument>();

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
      ReadAttr(rootNode, XmlVersion.data(), info.Version, "v0.4.4");
      m_version = info.Version;

      DeSerialize(info, rootNode);
    }
  }

  ResourceType Resource::GetType() const { return ResourceType::Base; }

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
    WriteAttr(refNode, doc, "Type", std::to_string((int) GetType()));

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
