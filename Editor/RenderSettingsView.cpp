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

          ImGui::EndDisabled();
        }

        if (ImGui::CollapsingHeader("Anti Aliasing"))
        {
          ImGui::Checkbox("FXAA##1", &gfx.FXAAEnabled);
        }

        ImGui::Separator();

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

      } // Imgui::Begin
      ImGui::End();
    }
  } // namespace Editor
} // namespace ToolKit