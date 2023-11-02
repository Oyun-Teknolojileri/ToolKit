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
    ReadAttr(node, "BloomEnabled", PostProcessing.BloomEnabled);
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
      Graphics.ShadowDistance = 50.0f;
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

} // namespace ToolKit