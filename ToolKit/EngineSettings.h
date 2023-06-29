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

#pragma once

#include "DofPass.h"
#include "ToneMapPass.h"

namespace ToolKit
{

  class TK_API EngineSettings : public Serializable
  {
   public:
    struct WindowSettings
    {
      String Name     = "ToolKit";
      uint Width      = 1024;
      uint Height     = 768;
      bool FullScreen = false;
    } Window;

    struct GraphicSettings
    {
      int MSAA = 2;
      int FPS  = 60;
    } Graphics;

    struct PostProcessingSettings
    {
      bool TonemappingEnabled      = true;
      TonemapMethod TonemapperMode = TonemapMethod::Aces;
      bool BloomEnabled            = true;
      float BloomIntensity         = 1.0f;
      float BloomThreshold         = 1.0f;
      int BloomIterationCount      = 5;
      bool GammaCorrectionEnabled  = true;
      float Gamma                  = 2.2f;
      bool SSAOEnabled             = true;
      float SSAORadius             = 0.5f;
      float SSAOBias               = 0.025f;
      float SSAOSpread             = 1.0f;
      bool DepthOfFieldEnabled     = false;
      float FocusPoint             = 10.0f;
      float FocusScale             = 5.0f;
      DoFQuality DofQuality        = DoFQuality::Normal;
      bool FXAAEnabled             = false;
    } PostProcessing;

    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerializeImp(XmlDocument* doc, XmlNode* parent) override;

    void SerializeWindow(XmlDocument* doc, XmlNode* parent) const;
    void DeSerializeWindow(XmlDocument* doc, XmlNode* parent);
    void SerializePostProcessing(XmlDocument* doc, XmlNode* parent) const;
    void DeSerializePostProcessing(XmlDocument* doc, XmlNode* parent);
    void SerializeGraphics(XmlDocument* doc, XmlNode* parent) const;
    void DeSerializeGraphics(XmlDocument* doc, XmlNode* parent);
  };

} // namespace ToolKit