/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "DofPass.h"
#include "Serialize.h"
#include "ToneMapPass.h"

namespace ToolKit
{
  enum class RenderingSpec
  {
    /** Deferred opaque + forward translucent path. High performance - High Bandwidth. */
    Default = 0,
    /** Forward renderer for all types. Low pass count and low bandwidth. */
    Mobile  = 1
  };

  class TK_API EngineSettings : public Serializable
  {
   public:
    struct WindowSettings
    {
      /** Application window name. */
      String Name     = "ToolKit";
      /** Application window width for windowed mode. */
      uint Width      = 1024;
      /** Application window height for windowed mode. */
      uint Height     = 768;
      /** States if the application is full screen or windowed. */
      bool FullScreen = false;

      void Serialize(XmlDocument* doc, XmlNode* parent) const;
      void DeSerialize(XmlDocument* doc, XmlNode* parent);
    } Window;

    struct GraphicSettings
    {
      /** Multi-sample count. 0 for non msaa render targets. */
      int MSAA                    = 0;

      /** Target fps for application. */
      int FPS                     = 60;

      /** Sets render targets as floating point, allows values larger than 1.0 for HDR rendering. */
      bool HDRPipeline            = true;

      /** Render path used for drawing. */
      RenderingSpec RenderSpec    = RenderingSpec::Default;

      /**
       * Viewport render target multiplier that adjusts the resolution.
       * High DPI devices such as mobile phones benefits from this.
       */
      float renderResolutionScale = 1.0f;

      /** Provides high precision gpu timers. Bad on cpu performance. Enable it only for profiling. */
      bool enableGpuTimer         = false;

      int cascadeCount            = 4;
      float cascadeDistances[4]   = {0.5f, 20.0f, 50.0f, 100.0f};

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
      float ShadowDistance         = 100.0f;
      int maxEntityPerBVHNode      = 5;
      float minBVHNodeSize         = 0.0f;

      void Serialize(XmlDocument* doc, XmlNode* parent) const;
      void DeSerialize(XmlDocument* doc, XmlNode* parent);
    } PostProcessing;

    StringArray LoadedPlugins;

    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

    void Save(const String& path);
    void Load(const String& path);
  };

} // namespace ToolKit