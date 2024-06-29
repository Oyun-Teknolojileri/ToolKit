/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "RenderSettingsWindow.h"

#include "App.h"

#include <BVH.h>
#include <EngineSettings.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {

    TKDefineClass(RenderSettingsWindow, Window);

    RenderSettingsWindow::RenderSettingsWindow() { m_name = g_renderSettings; }

    RenderSettingsWindow::~RenderSettingsWindow() {}

    void RenderSettingsWindow::Show()
    {
      EngineSettings& engineSettings = GetEngineSettings();

      ImGui::SetNextWindowSize(ImVec2(300, 600), ImGuiCond_Once);
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        EngineSettings::PostProcessingSettings& gfx = engineSettings.PostProcessing;

        ImGui::SeparatorText("Post Process");

        if (ImGui::CollapsingHeader("Tonemapping"))
        {
          ImGui::Checkbox("ToneMapping##1", &gfx.TonemappingEnabled);

          const char* items[] = {"Reinhard", "ACES"};
          uint itemCount      = sizeof(items) / sizeof(items[0]);
          uint tonemapperMode = (uint) gfx.TonemapperMode;
          if (ImGui::BeginCombo("Tonemapper mode", items[tonemapperMode]))
          {
            for (uint itemIndx = 0; itemIndx < itemCount; itemIndx++)
            {
              bool isSelected      = false;
              const char* itemName = items[itemIndx];
              ImGui::Selectable(itemName, &isSelected);
              if (isSelected)
              {
                gfx.TonemapperMode = (TonemapMethod) itemIndx;
              }
            }

            ImGui::EndCombo();
          }
        }

        if (ImGui::CollapsingHeader("Bloom"))
        {
          ImGui::Checkbox("Bloom##1", &gfx.BloomEnabled);

          ImGui::DragFloat("Bloom Intensity", &gfx.BloomIntensity, 0.01f, 0.0f, 100.0f);

          ImGui::DragFloat("Bloom Threshold", &gfx.BloomThreshold, 0.01f, 0.0f, FLT_MAX);

          ImGui::InputInt("Bloom Iteration Count", &gfx.BloomIterationCount, 1, 2);
        }

        if (ImGui::CollapsingHeader("Depth of Field"))
        {
          ImGui::Checkbox("Depth of Field##1", &gfx.DepthOfFieldEnabled);
          ImGui::BeginDisabled(!gfx.DepthOfFieldEnabled);
          ImGui::DragFloat("Focus Point", &gfx.FocusPoint, 0.1f, 0.0f, 100.0f);
          ImGui::DragFloat("Focus Scale", &gfx.FocusScale, 0.01f, 1.0f, 200.0f);

          const char* items[] = {"Low", "Normal", "High"};
          uint itemCount      = sizeof(items) / sizeof(items[0]);
          uint blurQuality    = (uint) gfx.DofQuality;
          if (ImGui::BeginCombo("Blur Quality", items[blurQuality]))
          {
            for (uint itemIndx = 0; itemIndx < itemCount; itemIndx++)
            {
              bool isSelected      = false;
              const char* itemName = items[itemIndx];
              ImGui::Selectable(itemName, &isSelected);
              if (isSelected)
              {
                gfx.DofQuality = (DoFQuality) itemIndx;
              }
            }

            ImGui::EndCombo();
          }
          ImGui::EndDisabled();
        }

        if (ImGui::CollapsingHeader("Ambient Occlusion"))
        {
          ImGui::Checkbox("SSAO##1", &gfx.SSAOEnabled);
          ImGui::BeginDisabled(!gfx.SSAOEnabled);

          ImGui::DragFloat("Radius", &gfx.SSAORadius, 0.001f, 0.0f, 1.0f);
          ImGui::DragFloat("Spread", &gfx.SSAOSpread, 0.001f, 0.0f, 1.0f);
          ImGui::DragFloat("Bias", &gfx.SSAOBias, 0.001f, 0.0f, 1.0f);
          ImGui::DragInt("KernelSize", &gfx.SSAOKernelSize, 1, 8, 128);

          ImGui::EndDisabled();
        }

        if (ImGui::CollapsingHeader("Anti Aliasing"))
        {
          ImGui::Checkbox("FXAA##1", &gfx.FXAAEnabled);
        }

        ImGui::SeparatorText("General");

        static bool lockFps = true;
        if (ImGui::Checkbox("FPS Lock##1", &lockFps))
        {
          if (lockFps)
          {
            Main::GetInstance()->m_timing.Init(engineSettings.Graphics.FPS);
          }
          else
          {
            Main::GetInstance()->m_timing.Init(9999);
          }
        }

        static bool multiThreaded = true;
        if (ImGui::Checkbox("MultiThread##1", &multiThreaded))
        {
          if (multiThreaded)
          {
            Main::GetInstance()->m_threaded = true;
          }
          else
          {
            Main::GetInstance()->m_threaded = false;
          }
        }

        bool hdrPipeline = engineSettings.Graphics.HDRPipeline;
        if (ImGui::Checkbox("HDR Pipeline##1", &hdrPipeline))
        {
          engineSettings.Graphics.HDRPipeline = hdrPipeline;
          g_app->ReInitViewports();
        }

        ImGui::SeparatorText("Multi Sample Anti Aliasing");

        auto showMsaaComboFn = [&engineSettings](std::function<void(int)>&& itemChangedFn) -> void
        {
          const char* itemNames[] = {"0", "2", "4"};
          const int itemCount     = sizeof(itemNames) / sizeof(itemNames[0]);
          int currentItem         = engineSettings.Graphics.msaa / 2;

          if (ImGui::BeginCombo("Sample Count", itemNames[currentItem]))
          {
            for (int itemIndx = 0; itemIndx < itemCount; itemIndx++)
            {
              bool isSelected      = false;
              const char* itemName = itemNames[itemIndx];
              ImGui::Selectable(itemName, &isSelected);
              if (isSelected)
              {
                itemChangedFn(itemIndx);
              }
            }

            ImGui::EndCombo();
          }
        };

        showMsaaComboFn(
            [&engineSettings](int newItem) -> void
            {
              engineSettings.Graphics.msaa = newItem * 2;
              g_app->ReInitViewports();
            });

        ImGui::SeparatorText("Shadows");

        bool* evsm4 = &engineSettings.Graphics.useEVSM4;
        if (ImGui::RadioButton("Use EVSM2", !*evsm4))
        {
          *evsm4 = false;
        }
        UI::AddTooltipToLastItem("Exponential variance shadow mapping with positive component.");

        ImGui::SameLine();

        if (ImGui::RadioButton("Use EVSM4", *evsm4))
        {
          *evsm4 = true;
        }
        UI::AddTooltipToLastItem("Exponential variance shadow mapping with positive and negative component."
                                 "\nRequires more shadow map memory, but yields softer shadows.");

        // Cascade count combo.
        {
          const char* itemNames[] = {"1", "2", "3", "4"};
          const int itemCount     = sizeof(itemNames) / sizeof(itemNames[0]);
          int currentItem         = engineSettings.Graphics.cascadeCount - 1;

          if (ImGui::BeginCombo("Cascade Count", itemNames[currentItem]))
          {
            for (int itemIndx = 0; itemIndx < itemCount; itemIndx++)
            {
              bool isSelected      = false;
              const char* itemName = itemNames[itemIndx];
              ImGui::Selectable(itemName, &isSelected);
              if (isSelected)
              {
                engineSettings.Graphics.cascadeCount = itemIndx + 1;
                GetRenderSystem()->InvalidateGPULightCache();
              }
            }

            ImGui::EndCombo();
          }
        }

        Vec4 data            = {engineSettings.Graphics.cascadeDistances[0],
                                engineSettings.Graphics.cascadeDistances[1],
                                engineSettings.Graphics.cascadeDistances[2],
                                engineSettings.Graphics.cascadeDistances[3]};

        int lastCascadeIndex = engineSettings.Graphics.cascadeCount - 1;
        Vec2 contentSize     = ImGui::GetContentRegionAvail();
        float width          = contentSize.x * 0.95f / 4.0f;
        width                = glm::clamp(width, 10.0f, 100.0f);

        bool manualSplit     = !engineSettings.Graphics.useParallelSplitPartitioning;
        ImGui::Checkbox("Manual Split Cascades", &manualSplit);
        engineSettings.Graphics.useParallelSplitPartitioning = !manualSplit;

        if (!manualSplit)
        {
          ImGui::BeginDisabled();
        }

        bool cascadeInvalidated = false;
        for (int i = 0; i < 4; i++)
        {
          float val = data[i];
          if (i > lastCascadeIndex)
          {
            ImGui::BeginDisabled();
            val = 0.0f;
          }

          ImGui::PushID(i);
          ImGui::PushItemWidth(width);

          if (ImGui::DragFloat("##cascade", &val))
          {
            cascadeInvalidated = true;
            data[i]            = val;
          }
          String msg = std::to_string(i + 1) + ". cascade distance";
          UI::AddTooltipToLastItem(msg.c_str());

          ImGui::PopItemWidth();
          ImGui::PopID();

          if (i > lastCascadeIndex)
          {
            ImGui::EndDisabled();
          }

          if (i < 3)
          {
            ImGui::SameLine();
          }
        }

        if (!manualSplit)
        {
          ImGui::EndDisabled();
        }

        if (cascadeInvalidated)
        {
          GetRenderSystem()->InvalidateGPULightCache();
          engineSettings.Graphics.cascadeDistances[0] = data.x;
          engineSettings.Graphics.cascadeDistances[1] = data.y;
          engineSettings.Graphics.cascadeDistances[2] = data.z;
          engineSettings.Graphics.cascadeDistances[3] = data.w;
        }

        ImGui::Checkbox("Parallel Split Cascades", &engineSettings.Graphics.useParallelSplitPartitioning);

        if (!engineSettings.Graphics.useParallelSplitPartitioning)
        {
          ImGui::BeginDisabled();
        }

        ImGui::DragFloat("Lambda", &engineSettings.Graphics.parallelSplitLambda, 0.01f, 0.0f, 1.0f, "%.2f");
        UI::AddTooltipToLastItem("Linear blending ratio between linear split and parallel split distances.");

        float shadowDistance = engineSettings.Graphics.GetShadowMaxDistance();
        ImGui::DragFloat("Shadow Distance", &shadowDistance, 10.0f, 0.0f, 10000.0f, "%.2f");

        engineSettings.Graphics.SetShadowMaxDistance(shadowDistance);

        if (!engineSettings.Graphics.useParallelSplitPartitioning)
        {
          ImGui::EndDisabled();
        }

        ImGui::Checkbox("Stabilize Shadows", &engineSettings.Graphics.stableShadowMap);
        UI::AddTooltipToLastItem("Prevents shimmering / swimming effects by wasting some shadow map resolution to "
                                 "prevent sub-pixel movements.");

        static bool highLightCascades = false;
        if (ImGui::Checkbox("Highlight Cascades", &highLightCascades))
        {
          ShaderPtr shader = GetShaderManager()->Create<Shader>(ShaderPath("defaultFragment.shader", true));
          shader->SetDefine("highlightCascades", highLightCascades ? "1" : "0");
        }
        UI::AddTooltipToLastItem("Highlights shadow cascades for debugging purpose.");

        ImGui::SeparatorText("Global Texture Settings");

        // Anisotropy combo.
        {
          const char* itemNames[] = {"0", "2", "4", "8", "16"};
          const int itemCount     = sizeof(itemNames) / sizeof(itemNames[0]);
          int currentItem         = engineSettings.Graphics.anisotropicTextureFiltering / 2;

          if (ImGui::BeginCombo("Anisotropy", itemNames[currentItem]))
          {
            for (int itemIndx = 0; itemIndx < itemCount; itemIndx++)
            {
              bool isSelected      = false;
              const char* itemName = itemNames[itemIndx];
              ImGui::Selectable(itemName, &isSelected);
              if (isSelected)
              {
                engineSettings.Graphics.anisotropicTextureFiltering = itemIndx * 2;
              }
            }

            ImGui::EndCombo();
          }
        }
        UI::AddTooltipToLastItem("Apply anisotropic filtering if the value is greater than 0. \nOnly effects all "
                                 "textures after editor restarted.");

        ImGui::SeparatorText("Global BVH Settings");
        ImGui::DragInt("Node Max Entity", &engineSettings.Graphics.maxEntityPerBVHNode, 1, 1, 1000000);
        ImGui::DragFloat("Node Min Size", &engineSettings.Graphics.minBVHNodeSize, 1.0f, 0.01f, 100000.0f);

        if (ImGui::Button("ReBuild BVH"))
        {
          if (ScenePtr scene = GetSceneManager()->GetCurrentScene())
          {
            if (BVHPtr bvh = GetSceneManager()->GetCurrentScene()->m_bvh)
            {
              bvh->ReBuild();

              int total = 0, dist = 0;
              float rat = 0.0f;
              bvh->DistributionQuality(total, dist, rat);
              TK_LOG("Total number of entities: %d, Entities in bvh: %d, Distribution: %f", total, dist, rat);
            }
          }
        }
      }
      ImGui::End();
    }

  } // namespace Editor
} // namespace ToolKit