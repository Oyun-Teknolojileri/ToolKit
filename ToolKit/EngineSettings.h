/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "DofPass.h"
#include "GammaTonemapFxaaPass.h"
#include "Serialize.h"

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
      /** Target fps for application. */
      int FPS                     = 60;

      /** Provides high precision gpu timers. Bad on cpu performance. Enable it only for profiling. */
      bool enableGpuTimer         = false;

      /** Multi-sample count. 0 for non msaa render targets. */
      int msaa                    = 0;

      /** Sets render targets as floating point, allows values larger than 1.0 for HDR rendering. */
      bool HDRPipeline            = true;

      /**
       * Viewport render target multiplier that adjusts the resolution.
       * High DPI devices such as mobile phones benefits from this.
       */
      float renderResolutionScale = 1.0f;

      /** Shadow cascade count. */
      int cascadeCount            = 4;

      /** Manual shadow cascade distances. */
      float cascadeDistances[4]   = {10.0f, 20.0f, 50.0f, 100.0f};

      float GetShadowMaxDistance() { return cascadeDistances[cascadeCount - 1]; }

      void SetShadowMaxDistance(float distance) { cascadeDistances[cascadeCount - 1] = distance; }

      float shadowMinDistance           = 0.01f;

      /**
       * Cascade splitting will either use manual cascadeDistances or calculated ones. If this is true cascadeDistances
       * are calculated as a mix between logarithmic and linear split.
       */
      bool useParallelSplitPartitioning = false;

      /** Linear mixture weight for parallel and linear splitting for cascades. */
      float parallelSplitLambda         = 1.0f;

      /** Prevents shimmering effects by preventing sub-pixel movement with the cost of wasted shadow map resolution. */
      bool stableShadowMap              = false;

      /** By default EVSM uses 2 component for shadow map generation. If this is true, it uses 4 component. */
      bool useEVSM4                     = false;

      /** Uses 32 bit shadow maps. */
      bool use32BitShadowMap            = true;

      /** Anisotropic texture filtering value. It can be 0, 2 ,4, 8, 16. Clamped with gpu max anisotropy. */
      int anisotropicTextureFiltering   = 8;

      /** Maximum number of entity count per bvh node. */
      int maxEntityPerBVHNode           = 5;

      /** Minimum size that a bvh node can be. */
      float minBVHNodeSize              = 0.0f;

      void Serialize(XmlDocument* doc, XmlNode* parent) const;
      void DeSerialize(XmlDocument* doc, XmlNode* parent);
    } Graphics;

    struct PostProcessingSettings
    {
      // Tone mapping
      /////////////////////
      bool TonemappingEnabled      = true;
      TonemapMethod TonemapperMode = TonemapMethod::Aces;

      // Bloom
      /////////////////////
      bool BloomEnabled            = true;
      float BloomIntensity         = 1.0f;
      float BloomThreshold         = 1.0f;
      int BloomIterationCount      = 5;

      // Gamma
      /////////////////////
      bool GammaCorrectionEnabled  = true;
      float Gamma                  = 2.2f;

      // SSAO
      /////////////////////
      bool SSAOEnabled             = true;
      float SSAORadius             = 0.5f;
      float SSAOBias               = 0.025f;
      float SSAOSpread             = 1.0f;
      int SSAOKernelSize           = 16;

      // DOF
      /////////////////////
      bool DepthOfFieldEnabled     = false;
      float FocusPoint             = 10.0f;
      float FocusScale             = 5.0f;
      DoFQuality DofQuality        = DoFQuality::Normal;

      // Anti-aliasing
      /////////////////////
      bool FXAAEnabled             = false;

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