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

  void EngineSettings::WindowSettings::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* window = doc->allocate_node(rapidxml::node_element, "Window");
    doc->append_node(window);

    using namespace std;

    const auto writeAttr1 = [&](StringView name, StringView val) { WriteAttr(window, doc, name.data(), val.data()); };
    // serialize window.
    writeAttr1("width", to_string(Width));
    writeAttr1("height", to_string(Height));
    writeAttr1(XmlNodeName, Name);
    writeAttr1("fullscreen", to_string(FullScreen));
  }

  void EngineSettings::WindowSettings::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    XmlNode* node = doc->first_node("Window");
    if (node == nullptr)
    {
      return;
    }
    ReadAttr(node, "width", Width);
    ReadAttr(node, "height", Height);
    ReadAttr(node, "name", Name);
    ReadAttr(node, "fullscreen", FullScreen);
  }

  void EngineSettings::PostProcessingSettings::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* settings = doc->allocate_node(rapidxml::node_element, "PostProcessing");
    doc->append_node(settings);

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
  }

  void EngineSettings::PostProcessingSettings::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    XmlNode* node = doc->first_node("PostProcessing");
    // if post processing settings is not exist use default settings
    if (node == nullptr)
    {
      return;
    }

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
  }

  void EngineSettings::GraphicSettings::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* settings = doc->allocate_node(rapidxml::node_element, "Graphics");
    doc->append_node(settings);

    WriteAttr(settings, doc, "MSAA", std::to_string(MSAA));
    WriteAttr(settings, doc, "FPS", std::to_string(FPS));
    WriteAttr(settings, doc, "HDRPipeline", std::to_string(HDRPipeline));
    WriteAttr(settings, doc, "RenderSpec", std::to_string((int) RenderSpec));
    WriteAttr(settings, doc, "RenderResolutionScale", std::to_string(renderResolutionScale));
  }

  void EngineSettings::GraphicSettings::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    XmlNode* node = doc->first_node("Graphics");
    if (node == nullptr)
    {
      return;
    }
    ReadAttr(node, "MSAA", MSAA);
    ReadAttr(node, "FPS", FPS);
    ReadAttr(node, "HDRPipeline", HDRPipeline);
    ReadAttr(node, "RenderResolutionScale", renderResolutionScale);
    if (renderResolutionScale < 0.01f)
    {
      renderResolutionScale = 1.0f;
    }

    int renderSpec;
    ReadAttr(node, "RenderSpec", renderSpec);
    RenderSpec = (RenderingSpec) renderSpec;
  }

  XmlNode* EngineSettings::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    Graphics.Serialize(doc, parent);
    PostProcessing.Serialize(doc, parent);
    Window.Serialize(doc, parent);

    return nullptr;
  }

  XmlNode* EngineSettings::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    Window.DeSerialize(info.Document, parent);
    Graphics.DeSerialize(info.Document, parent);
    PostProcessing.DeSerialize(info.Document, parent);

    return nullptr;
  }

  void EngineSettings::Save(const String& path)
  {
    std::ofstream file;
    file.open(path.c_str(), std::ios::out | std::ios::trunc);
    assert(file.is_open());
    if (file.is_open())
    {
      XmlDocumentPtr lclDoc = MakeNewPtr<XmlDocument>();
      XmlDocument* doc      = lclDoc.get();

      // Always write the current version.
      XmlNode* version      = CreateXmlNode(doc, "Version");
      WriteAttr(version, doc, "version", m_version);

      Window.Serialize(doc, nullptr);
      Graphics.Serialize(doc, nullptr);

      std::string xml;
      rapidxml::print(std::back_inserter(xml), *lclDoc);
      file << xml;
      file.close();
      lclDoc->clear();
    }
  }

  void EngineSettings::Load(const String& path)
  {
    XmlFilePtr lclFile    = MakeNewPtr<XmlFile>(path.c_str());
    XmlDocumentPtr lclDoc = MakeNewPtr<XmlDocument>();
    lclDoc->parse<0>(lclFile->data());

    Window.DeSerialize(lclDoc.get(), nullptr);
    Graphics.DeSerialize(lclDoc.get(), nullptr);
  }

} // namespace ToolKit