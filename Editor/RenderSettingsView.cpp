#include "RenderSettingsView.h"

#include "App.h"

namespace ToolKit
{
  namespace Editor
  {
    RenderSettingsView::RenderSettingsView() : View("Render Settings View")
    {
      m_viewID  = 5;
      m_viewIcn = UI::m_sceneIcon;
    }

    RenderSettingsView::~RenderSettingsView() {}

    void RenderSettingsView::Show()
    {
      EngineSettings::GraphicSettings& gfx =
          Main::GetInstance()->m_engineSettings.Graphics;
      if (gfx.TonemappingEnabled && ImGui::CollapsingHeader("Tonemapping"))
      {
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

      if (gfx.BloomEnabled && ImGui::CollapsingHeader("Bloom"))
      {
        ImGui::DragFloat("Bloom Intensity",
                         &gfx.BloomIntensity,
                         0.01f,
                         0.0f,
                         100.0f);
        ImGui::DragFloat("Bloom Threshold",
                         &gfx.BloomThreshold,
                         0.01f,
                         0.0f,
                         FLT_MAX);
        ImGui::InputInt("Bloom Iteration Count",
                        &gfx.BloomIterationCount,
                        1,
                        2);
      }

      if (ImGui::CollapsingHeader("Depth of Field"))
      {
        ImGui::Checkbox("Depth of Field##1", &gfx.DepthofFieldEnabled);
        ImGui::BeginDisabled(!gfx.DepthofFieldEnabled);
        ImGui::DragFloat("Focus Point", &gfx.focusPoint, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Focus Scale", &gfx.focusScale, 0.01f, 1.0f, 200.0f);

        const char* items[] = {"Low", "Normal", "High"};
        uint itemCount      = sizeof(items) / sizeof(items[0]);
        uint blurQuality    = (uint) gfx.dofQuality;
        if (ImGui::BeginCombo("Blur Quality", items[blurQuality]))
        {
          for (uint itemIndx = 0; itemIndx < itemCount; itemIndx++)
          {
            bool isSelected      = false;
            const char* itemName = items[itemIndx];
            ImGui::Selectable(itemName, &isSelected);
            if (isSelected)
            {
              gfx.dofQuality = (DoFQuality) itemIndx;
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
        ImGui::InputFloat("Radius", &gfx.ssaoRadius, 0.01f);
        ImGui::InputFloat("Bias", &gfx.ssaoBias, 0.001f);
        ImGui::EndDisabled();
      }

      if (ImGui::CollapsingHeader("Anti Aliasing"))
      {
        ImGui::Checkbox("FXAA##1", &gfx.FXAAEnabled);
      }
    }
  } // namespace Editor
} // namespace ToolKit