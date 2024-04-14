/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "EngineSettings.h"

#include "MathUtil.h"
#include "PluginManager.h"
#include "ToolKit.h"



namespace ToolKit
{

  void EngineSettings::WindowSettings::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* window = CreateXmlNode(doc, "Window", parent);
    WriteAttr(window, doc, XmlNodeName, Name);
    WriteAttr(window, doc, "width", std::to_string(Width));
    WriteAttr(window, doc, "height", std::to_string(Height));
    WriteAttr(window, doc, "fullscreen", std::to_string(FullScreen));
  }

  void EngineSettings::WindowSettings::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    if (XmlNode* node = parent->first_node("Window"))
    {
      ReadAttr(node, XmlNodeName.data(), Name);
      ReadAttr(node, "width", Width);
      ReadAttr(node, "height", Height);
      ReadAttr(node, "fullscreen", FullScreen);
    }
  }

  void EngineSettings::PostProcessingSettings::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* settings      = CreateXmlNode(doc, "PostProcessing", parent);

    const auto writeAttrFn = [&](StringView name, StringView val) -> void
    { WriteAttr(settings, doc, name.data(), val.data()); };

    // Serialize Graphics struct.
    using namespace std;
    writeAttrFn("TonemappingEnabled", to_string(TonemappingEnabled));
    writeAttrFn("TonemapperMode", to_string((int) TonemapperMode));
    writeAttrFn("EnableBloom", to_string((int) BloomEnabled));
    writeAttrFn("BloomIntensity", to_string(BloomIntensity));
    writeAttrFn("BloomThreshold", to_string(BloomThreshold));
    writeAttrFn("BloomIterationCount", to_string(BloomIterationCount));
    writeAttrFn("GammaCorrectionEnabled", to_string(GammaCorrectionEnabled));
    writeAttrFn("Gamma", to_string(Gamma));
    writeAttrFn("SSAOEnabled", to_string(SSAOEnabled));
    writeAttrFn("SSAORadius", to_string(SSAORadius));
    writeAttrFn("SSAOBias", to_string(SSAOBias));
    writeAttrFn("SSAOKernelSize", to_string(SSAOKernelSize));
    writeAttrFn("DepthOfFieldEnabled", to_string(DepthOfFieldEnabled));
    writeAttrFn("FocusPoint", to_string(FocusPoint));
    writeAttrFn("FocusScale", to_string(FocusScale));
    writeAttrFn("DofQuality", to_string((int) DofQuality));
    writeAttrFn("FXAAEnabled", to_string(FXAAEnabled));
    writeAttrFn("ShadowDistance", to_string(ShadowDistance));
    writeAttrFn("MaxEntityPerBVHNode", to_string(maxEntityPerBVHNode));
    writeAttrFn("MinBVHNodeSize", to_string(minBVHNodeSize));
  }

  void EngineSettings::PostProcessingSettings::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    // if post processing settings is not exist use default settings
    if (parent == nullptr)
    {
      return;
    }

    if (XmlNode* node = parent->first_node("PostProcessing"))
    {
      ReadAttr(node, "TonemappingEnabled", TonemappingEnabled);
      ReadAttr(node, "EnableBloom", BloomEnabled);
      ReadAttr(node, "BloomIntensity", BloomIntensity);
      ReadAttr(node, "BloomThreshold", BloomThreshold);
      ReadAttr(node, "BloomIterationCount", BloomIterationCount);
      ReadAttr(node, "GammaCorrectionEnabled", GammaCorrectionEnabled);
      ReadAttr(node, "Gamma", Gamma);
      ReadAttr(node, "SSAOEnabled", SSAOEnabled);
      ReadAttr(node, "SSAORadius", SSAORadius);
      ReadAttr(node, "SSAOBias", SSAOBias);
      ReadAttr(node, "SSAOKernelSize", SSAOKernelSize);
      ReadAttr(node, "DepthOfFieldEnabled", DepthOfFieldEnabled);
      ReadAttr(node, "FocusPoint", FocusPoint);
      ReadAttr(node, "FocusScale", FocusScale);
      ReadAttr(node, "FXAAEnabled", FXAAEnabled);
      ReadAttr(node, "TonemapperMode", *(int*) &TonemapperMode);
      ReadAttr(node, "DofQuality", *(int*) &DofQuality);
      ReadAttr(node, "ShadowDistance", ShadowDistance);
      ReadAttr(node, "MaxEntityPerBVHNode", maxEntityPerBVHNode);
      ReadAttr(node, "MinBVHNodeSize", minBVHNodeSize);
    }
  }

  void EngineSettings::GraphicSettings::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* settings = CreateXmlNode(doc, "Graphics", parent);
    WriteAttr(settings, doc, "MSAA", std::to_string(MSAA));
    WriteAttr(settings, doc, "FPS", std::to_string(FPS));
    WriteAttr(settings, doc, "HDRPipeline", std::to_string(HDRPipeline));
    WriteAttr(settings, doc, "RenderSpec", std::to_string((int) RenderSpec));
    WriteAttr(settings, doc, "RenderResolutionScale", std::to_string(renderResolutionScale));
    WriteAttr(settings, doc, "EnableGpuTimer", std::to_string(enableGpuTimer));
  }

  void EngineSettings::GraphicSettings::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    if (XmlNode* node = parent->first_node("Graphics"))
    {
      ReadAttr(node, "MSAA", MSAA);
      ReadAttr(node, "FPS", FPS);
      ReadAttr(node, "HDRPipeline", HDRPipeline);
      ReadAttr(node, "RenderResolutionScale", renderResolutionScale);
      ReadAttr(node, "RenderSpec", *(int*) &RenderSpec);
      ReadAttr(node, "EnableGpuTimer", enableGpuTimer);
    }
  }

  XmlNode* EngineSettings::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    if (doc == nullptr)
    {
      return parent;
    }

    XmlNode* settingsNode = CreateXmlNode(doc, "Settings", nullptr);
    WriteAttr(settingsNode, doc, "version", TKVersionStr);

    Window.Serialize(doc, settingsNode);
    Graphics.Serialize(doc, settingsNode);

    XmlNode* pluginNode = CreateXmlNode(doc, "Plugins", settingsNode);

    if (PluginManager* plugMan = GetPluginManager())
    {
      for (const PluginRegister& reg : plugMan->m_storage)
      {
        if (reg.m_loaded)
        {
          if (reg.m_plugin->GetType() != PluginType::Game)
          {
            XmlNode* plugin = CreateXmlNode(doc, "Plugin", pluginNode);
            WriteAttr(plugin, doc, "name", reg.m_name);
          }
        }
      }
    }

    return settingsNode;
  }

  XmlNode* EngineSettings::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlDocument* doc      = info.Document;
    XmlNode* settingsNode = doc->first_node("Settings");

    Window.DeSerialize(doc, settingsNode);
    Graphics.DeSerialize(doc, settingsNode);

    if (XmlNode* pluginNode = settingsNode->first_node("Plugins"))
    {
      XmlNode* plugin = pluginNode->first_node();
      while (plugin)
      {
        String pluginName;
        ReadAttr(plugin, "name", pluginName);
        LoadedPlugins.push_back(pluginName);

        plugin = plugin->next_sibling();
      }
    }

    return settingsNode;
  }

  void EngineSettings::Save(const String& path)
  {
    std::ofstream file;
    file.open(path.c_str(), std::ios::out | std::ios::trunc);
    assert(file.is_open());

    if (file.is_open())
    {
      XmlDocument* lclDoc = new XmlDocument();
      SerializeImp(lclDoc, nullptr);

      std::string xml;
      rapidxml::print(std::back_inserter(xml), *lclDoc);
      file << xml;
      file.close();
      lclDoc->clear();

      SafeDel(lclDoc);
    }
  }

  void EngineSettings::Load(const String& path)
  {
    XmlFile* lclFile    = new XmlFile(path.c_str());
    XmlDocument* lclDoc = new XmlDocument();
    lclDoc->parse<0>(lclFile->data());

    SerializationFileInfo info;
    info.File     = path;
    info.Document = lclDoc;

    DeSerializeImp(info, nullptr);

    SafeDel(lclFile);
    SafeDel(lclDoc);
  }

} // namespace ToolKit