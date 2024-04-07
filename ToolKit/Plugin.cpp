/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Plugin.h"

namespace ToolKit
{

  void PluginSettings::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    if (doc == nullptr)
    {
      return;
    }

    XmlNode* pluginNode = CreateXmlNode(doc, "Settings", parent);
    XmlNode* node       = CreateXmlNode(doc, "Plugin", pluginNode);
    WriteAttr(node, doc, "autoLoad", std::to_string(autoLoad));
    WriteAttr(node, doc, "version", version);
    WriteAttr(node, doc, "engine", engine);

    node = CreateXmlNode(doc, "Description", pluginNode);
    WriteAttr(node, doc, "name", name);
    WriteAttr(node, doc, "brief", brief);

    node = CreateXmlNode(doc, "Developer", pluginNode);
    WriteAttr(node, doc, "name", developer);
    WriteAttr(node, doc, "web", web);
    WriteAttr(node, doc, "email", email);
  }

  void PluginSettings::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (doc == nullptr)
    {
      return;
    }

    if (XmlNode* settings = doc->first_node("Settings"))
    {
      if (XmlNode* plugin = settings->first_node("Plugin"))
      {
        ReadAttr(plugin, "autoLoad", autoLoad);
        ReadAttr(plugin, "version", version);
        ReadAttr(plugin, "engine", engine);
      }

      if (XmlNode* desc = settings->first_node("Description"))
      {
        ReadAttr(desc, "name", name);
        ReadAttr(desc, "brief", brief);

        file = PluginPath(name);
      }

      if (XmlNode* dev = settings->first_node("Developer"))
      {
        ReadAttr(dev, "name", developer);
        ReadAttr(dev, "web", web);
        ReadAttr(dev, "email", email);
      }
    }
  }

  void PluginSettings::Load(const String& path)
  {
    XmlFile* lclFile    = new XmlFile(path.c_str());
    XmlDocument* lclDoc = new XmlDocument();
    lclDoc->parse<0>(lclFile->data());

    DeSerialize(lclDoc, nullptr);

    SafeDel(lclFile);
    SafeDel(lclDoc);
  }

  void PluginSettings::Save(const String& path)
  {
    std::ofstream file;
    file.open(path.c_str(), std::ios::out | std::ios::trunc);
    assert(file.is_open());

    if (file.is_open())
    {
      XmlDocument* lclDoc = new XmlDocument();
      Serialize(lclDoc, nullptr);

      std::string xml;
      rapidxml::print(std::back_inserter(xml), *lclDoc);
      file << xml;
      file.close();
      lclDoc->clear();

      SafeDel(lclDoc);
    }
  }

} // namespace ToolKit