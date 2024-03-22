/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "DofPass.h"
#include "ToneMapPass.h"

namespace ToolKit
{
  enum class RenderingSpec
  {
    // Make sure there is no gap between integer values
    Default = 0,
    Mobile  = 1
  };

  class TK_API EngineSettings : public Serializable
  {
   public:
    struct WindowSettings
    {
      String Name     = "ToolKit";
      uint Width      = 1024;
      uint Height     = 768;
      bool FullScreen = false;

      void Serialize(XmlDocument* doc, XmlNode* parent) const;
      void DeSerialize(XmlDocument* doc, XmlNode* parent);
    } Window;

    struct GraphicSettings
    {
      int MSAA                    = 2;
      int FPS                     = 60;
      bool HDRRender              = true;
      RenderingSpec RenderSpec    = RenderingSpec::Default;
      float ShadowDistance        = 100.0f;
      float renderResolutionScale = 1.0f;

      void Serialize(XmlDocument* doc, XmlNode* parent) const;
      void DeSerialize(XmlDocument* doc, XmlNode* parent);
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
      int SSAOKernelSize           = 16;
      bool DepthOfFieldEnabled     = false;
      float FocusPoint             = 10.0f;
      float FocusScale             = 5.0f;
      DoFQuality DofQuality        = DoFQuality::Normal;
      bool FXAAEnabled             = false;

      void Serialize(XmlDocument* doc, XmlNode* parent) const;
      void DeSerialize(XmlDocument* doc, XmlNode* parent);
    } PostProcessing;

    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

    void Save(const String& path);
    void Load(const String& path);
  };

} // namespace ToolKit