#include "RenderSettingsView.h"

#include "App.h"

namespace ToolKit
{
  namespace Editor
  {
    RenderSettingsView::RenderSettingsView() 
    {
      m_name = g_renderSettings;
    }

    RenderSettingsView::~RenderSettingsView() {}
    
    Window::Type RenderSettingsView::GetType() const
    {
      return Window::Type::RenderSettings;
    }

    void RenderSettingsView::Show()
    {
      ImGui::SetNextWindowSize(ImVec2(300, 600), ImGuiCond_Once);
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        EngineSettings::PostProcessingSettings& gfx =
            Main::GetInstance()->m_engineSettings.PostProcessing;
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
      } // Imgui::Begin
      ImGui::End();
    }
  } // namespace Editor
} // namespace ToolKit