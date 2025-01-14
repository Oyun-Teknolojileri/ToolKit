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
    }
  }

  void EngineSettings::GraphicSettings::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* settings = CreateXmlNode(doc, "Graphics", parent);
    WriteAttr(settings, doc, "MSAA", std::to_string(msaa));
    WriteAttr(settings, doc, "FPS", std::to_string(FPS));
    WriteAttr(settings, doc, "HDRPipeline", std::to_string(HDRPipeline));
    WriteAttr(settings, doc, "RenderResolutionScale", std::to_string(renderResolutionScale));
    WriteAttr(settings, doc, "EnableGpuTimer", std::to_string(enableGpuTimer));

    WriteAttr(settings, doc, "CascadeCount", std::to_string(cascadeCount));
    WriteAttr(settings, doc, "CascacdeDist0", std::to_string(cascadeDistances[0]));
    WriteAttr(settings, doc, "CascacdeDist1", std::to_string(cascadeDistances[1]));
    WriteAttr(settings, doc, "CascacdeDist2", std::to_string(cascadeDistances[2]));
    WriteAttr(settings, doc, "CascacdeDist3", std::to_string(cascadeDistances[3]));

    WriteAttr(settings, doc, "UseEvsm4", std::to_string(useEVSM4));
    WriteAttr(settings, doc, "UsePSSM", std::to_string(useParallelSplitPartitioning));
    WriteAttr(settings, doc, "PSSMLambda", std::to_string(parallelSplitLambda));
    WriteAttr(settings, doc, "StableShadow", std::to_string(stableShadowMap));
    WriteAttr(settings, doc, "Use32BitSM", std::to_string(use32BitShadowMap));

    WriteAttr(settings, doc, "AnisotropicTextureFiltering", std::to_string(anisotropicTextureFiltering));

    WriteAttr(settings, doc, "MaxEntityPerBVHNode", std::to_string(maxEntityPerBVHNode));
    WriteAttr(settings, doc, "MinBVHNodeSize", std::to_string(minBVHNodeSize));
  }

  void EngineSettings::GraphicSettings::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      return;
    }

    if (XmlNode* node = parent->first_node("Graphics"))
    {
      ReadAttr(node, "MSAA", msaa);
      ReadAttr(node, "FPS", FPS);
      ReadAttr(node, "HDRPipeline", HDRPipeline);
      ReadAttr(node, "RenderResolutionScale", renderResolutionScale);
      ReadAttr(node, "EnableGpuTimer", enableGpuTimer);

      ReadAttr(node, "CascadeCount", cascadeCount);
      ReadAttr(node, "CascacdeDist0", cascadeDistances[0]);
      ReadAttr(node, "CascacdeDist1", cascadeDistances[1]);
      ReadAttr(node, "CascacdeDist2", cascadeDistances[2]);
      ReadAttr(node, "CascacdeDist3", cascadeDistances[3]);

      ReadAttr(node, "UseEvsm4", useEVSM4);
      ReadAttr(node, "UsePSSM", useParallelSplitPartitioning);
      ReadAttr(node, "PSSMLambda", parallelSplitLambda);
      ReadAttr(node, "StableShadow", stableShadowMap);
      ReadAttr(node, "Use32BitSM", use32BitShadowMap);

      ReadAttr(node, "AnisotropicTextureFiltering", anisotropicTextureFiltering);

      ReadAttr(node, "MaxEntityPerBVHNode", maxEntityPerBVHNode);
      ReadAttr(node, "MinBVHNodeSize", minBVHNodeSize);
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

    // TODO: This data does not need to be read, write always. It can be disabled enabled based on a config.
    XmlNode* profileTimerNode = CreateXmlNode(doc, "ProfileTimer", settingsNode);
    for (const auto& timer : TKStatTimerMap)
    {
      WriteAttr(profileTimerNode, doc, timer.first, std::to_string(timer.second.enabled));
    }

    return settingsNode;
  }

  XmlNode* EngineSettings::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlDocument* doc      = info.Document;
    XmlNode* settingsNode = doc->first_node("Settings");

    Window.DeSerialize(doc, settingsNode);
    Graphics.DeSerialize(doc, settingsNode);

    if (XmlNode* timerNode = settingsNode->first_node("ProfileTimer"))
    {
      XmlAttribute* timer = timerNode->first_attribute();
      while (timer)
      {
        TKStatTimerMap[timer->name()].enabled = (bool) std::atoi(timer->value());
        timer                                 = timer->next_attribute();
      }
    }

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