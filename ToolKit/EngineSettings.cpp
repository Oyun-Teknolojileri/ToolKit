/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "EngineSettings.h"

#include "MathUtil.h"

#include "DebugNew.h"

namespace ToolKit
{

  void EngineSettings::SerializeWindow(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* window = doc->allocate_node(rapidxml::node_element, "Window");
    doc->append_node(window);

    using namespace std;
    const EngineSettings::GraphicSettings& gfx = Graphics;

    const auto writeAttr1 = [&](StringView name, StringView val) { WriteAttr(window, doc, name.data(), val.data()); };
    // serialize window.
    writeAttr1("width", to_string(Window.Width));
    writeAttr1("height", to_string(Window.Height));
    writeAttr1(XmlNodeName, Window.Name);
    writeAttr1("fullscreen", to_string(Window.FullScreen));
  }

  void EngineSettings::DeSerializeWindow(XmlDocument* doc, XmlNode* parent)
  {
    XmlNode* node = doc->first_node("Window");
    if (node == nullptr)
    {
      return;
    }
    ReadAttr(node, "width", Window.Width);
    ReadAttr(node, "height", Window.Height);
    ReadAttr(node, "name", Window.Name);
    ReadAttr(node, "fullscreen", Window.FullScreen);
  }

  void EngineSettings::SerializePostProcessing(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* settings = doc->allocate_node(rapidxml::node_element, "PostProcessing");
    doc->append_node(settings);

    const EngineSettings::PostProcessingSettings& gfx = PostProcessing;

    const auto writeAttrFn                            = [&](StringView name, StringView val) -> void
    { WriteAttr(settings, doc, name.data(), val.data()); };

    // Serialize Graphics struct.
    using namespace std;
    writeAttrFn("TonemappingEnabled", to_string(gfx.TonemappingEnabled));
    writeAttrFn("TonemapperMode", to_string((int) gfx.TonemapperMode));
    writeAttrFn("EnableBloom", to_string((int) gfx.BloomEnabled));
    writeAttrFn("BloomIntensity", to_string(gfx.BloomIntensity));
    writeAttrFn("BloomThreshold", to_string(gfx.BloomThreshold));
    writeAttrFn("BloomIterationCount", to_string(gfx.BloomIterationCount));
    writeAttrFn("GammaCorrectionEnabled", to_string(gfx.GammaCorrectionEnabled));
    writeAttrFn("Gamma", to_string(gfx.Gamma));
    writeAttrFn("SSAOEnabled", to_string(gfx.SSAOEnabled));
    writeAttrFn("SSAORadius", to_string(gfx.SSAORadius));
    writeAttrFn("SSAOBias", to_string(gfx.SSAOBias));
    writeAttrFn("SSAOKernelSize", to_string(gfx.SSAOKernelSize));
    writeAttrFn("DepthOfFieldEnabled", to_string(gfx.DepthOfFieldEnabled));
    writeAttrFn("FocusPoint", to_string(gfx.FocusPoint));
    writeAttrFn("FocusScale", to_string(gfx.FocusScale));
    writeAttrFn("DofQuality", to_string((int) gfx.DofQuality));
    writeAttrFn("FXAAEnabled", to_string(gfx.FXAAEnabled));

    // Additional graphics settings per scene.
    writeAttrFn("ShadowDistance", to_string(Graphics.ShadowDistance));
  }

  void EngineSettings::DeSerializePostProcessing(XmlDocument* doc, XmlNode* parent)
  {
    XmlNode* node  = doc->first_node("PostProcessing");
    // if post processing settings is not exist use default settings
    PostProcessing = PostProcessingSettings {};
    if (node == nullptr)
    {
      return;
    }

    ReadAttr(node, "TonemappingEnabled", PostProcessing.TonemappingEnabled);
    ReadAttr(node, "EnableBloom", PostProcessing.BloomEnabled);
    ReadAttr(node, "BloomIntensity", PostProcessing.BloomIntensity);
    ReadAttr(node, "BloomThreshold", PostProcessing.BloomThreshold);
    ReadAttr(node, "BloomIterationCount", PostProcessing.BloomIterationCount);
    ReadAttr(node, "GammaCorrectionEnabled", PostProcessing.GammaCorrectionEnabled);
    ReadAttr(node, "Gamma", PostProcessing.Gamma);
    ReadAttr(node, "SSAOEnabled", PostProcessing.SSAOEnabled);
    ReadAttr(node, "SSAORadius", PostProcessing.SSAORadius);
    ReadAttr(node, "SSAOBias", PostProcessing.SSAOBias);
    ReadAttr(node, "SSAOKernelSize", PostProcessing.SSAOKernelSize);
    ReadAttr(node, "DepthOfFieldEnabled", PostProcessing.DepthOfFieldEnabled);
    ReadAttr(node, "FocusPoint", PostProcessing.FocusPoint);
    ReadAttr(node, "FocusScale", PostProcessing.FocusScale);
    ReadAttr(node, "FXAAEnabled", PostProcessing.FXAAEnabled);
    ReadAttr(node, "TonemapperMode", *(int*) &PostProcessing.TonemapperMode);
    ReadAttr(node, "DofQuality", *(int*) &PostProcessing.DofQuality);

    // Additional graphics settings per scene.
    ReadAttr(node, "ShadowDistance", Graphics.ShadowDistance);
  }

  void EngineSettings::SerializeGraphics(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* settings = doc->allocate_node(rapidxml::node_element, "Graphics");
    doc->append_node(settings);
    const EngineSettings::GraphicSettings& gfx = Graphics;

    WriteAttr(settings, doc, "MSAA", std::to_string(gfx.MSAA));
    WriteAttr(settings, doc, "FPS", std::to_string(gfx.FPS));
    WriteAttr(settings, doc, "RenderSpec", std::to_string((int) gfx.RenderSpec));
    WriteAttr(settings, doc, "ShadowDistance", std::to_string(gfx.ShadowDistance));
  }

  void EngineSettings::DeSerializeGraphics(XmlDocument* doc, XmlNode* parent)
  {
    XmlNode* node = doc->first_node("Graphics");
    if (node == nullptr)
    {
      return;
    }
    ReadAttr(node, "MSAA", Graphics.MSAA);
    ReadAttr(node, "FPS", Graphics.FPS);
    ReadAttr(node, "ShadowDistance", Graphics.ShadowDistance);
    if (!glm::epsilonNotEqual(Graphics.ShadowDistance, 0.0f, 0.001f))
    {
      // Set the value to the default value if the variable is not deserialized
      Graphics.ShadowDistance = 100.0f;
    }

    int renderSpec;
    ReadAttr(node, "RenderSpec", renderSpec);
    Graphics.RenderSpec = (RenderingSpec) renderSpec;
  }

  XmlNode* EngineSettings::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    SerializeGraphics(doc, parent);
    SerializePostProcessing(doc, parent);
    SerializeWindow(doc, parent);

    return nullptr;
  }

  XmlNode* EngineSettings::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    DeSerializeWindow(info.Document, parent);
    DeSerializeGraphics(info.Document, parent);
    DeSerializePostProcessing(info.Document, parent);

    return nullptr;
  }

    void EngineSettings::SerializeEngineSettings(const String& engineSettingsFilePath)
  {
    std::ofstream file;
    const String& path = engineSettingsFilePath;

    file.open(path.c_str(), std::ios::out | std::ios::trunc);
    assert(file.is_open());
    if (file.is_open())
    {
      XmlDocument* lclDoc = new XmlDocument();

      // Always write the current version.
      XmlNode* version    = lclDoc->allocate_node(rapidxml::node_element, "Version");
      lclDoc->append_node(version);
      WriteAttr(version, lclDoc, "version", TKVersionStr);

      SerializeWindow(lclDoc, nullptr);
      SerializeGraphics(lclDoc, nullptr);
      // TODO move this data to editor settings
      // SerializeSimulationWindow(lclDoc);

      std::string xml;
      rapidxml::print(std::back_inserter(xml), *lclDoc);
      file << xml;
      file.close();
      lclDoc->clear();

      SafeDel(lclDoc);
    }
  }

  void EngineSettings::DeSerializeEngineSettings(const String& engineSettingsFilePath)
  {
    const String& settingsFile = engineSettingsFilePath;
    XmlFile* lclFile           = new XmlFile(settingsFile.c_str());
    XmlDocument* lclDoc        = new XmlDocument();
    lclDoc->parse<0>(lclFile->data());

    DeSerializeWindow(lclDoc, nullptr);
    DeSerializeGraphics(lclDoc, nullptr);

    // TODO move this data to editor settings
    // DeSerializeSimulationWindow(lclDoc);

    SafeDel(lclFile);
    SafeDel(lclDoc);
  }

} // namespace ToolKit