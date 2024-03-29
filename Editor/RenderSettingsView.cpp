/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "RenderSettingsView.h"

#include "App.h"

#include <EngineSettings.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {
    RenderSettingsView::RenderSettingsView() { m_name = g_renderSettings; }

    RenderSettingsView::~RenderSettingsView() {}

    Window::Type RenderSettingsView::GetType() const { return Window::Type::RenderSettings; }

    void RenderSettingsView::Show()
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

        const char* renderSpecNames[] = {"High", "Low"};
        const int currentRenderSpec   = (int) engineSettings.Graphics.RenderSpec;
        const int specCount           = 2;
        if (ImGui::BeginCombo("Rendering Spec", renderSpecNames[currentRenderSpec]))
        {
          for (uint specIndex = 0; specIndex < specCount; specIndex++)
          {
            bool isSelected      = false;
            const char* itemName = renderSpecNames[specIndex];
            ImGui::Selectable(itemName, &isSelected);
            if (isSelected)
            {
              engineSettings.Graphics.RenderSpec = (RenderingSpec) specIndex;
            }
          }
          ImGui::EndCombo();
        }

        ImGui::DragFloat("Shadow Distance", &engineSettings.Graphics.ShadowDistance, 1.0f, 0.01f, 100000.0f);

      } // Imgui::Begin
      ImGui::End();
    }
  } // namespace Editor
} // namespace ToolKit