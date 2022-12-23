#include "RenderSettingsView.h"

#include "App.h"

namespace ToolKit
{
  namespace Editor
  {
    RenderSettingsView::RenderSettingsView() : View("Render Settings View")
    {
      m_viewID  = 5;
      m_viewIcn = UI::m_cameraIcon;
    }

    RenderSettingsView::~RenderSettingsView() {}

    void RenderSettingsView::Show()
    {
      const char* items[] = {"Reinhard", "ACES"};
      uint itemCount      = sizeof(items) / sizeof(items[0]);
      EngineSettings::GraphicSettings& gfx =
          Main::GetInstance()->m_engineSettings.Graphics;
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

      ImGui::DragFloat("Bloom Intensity",
                       &gfx.bloomIntensity,
                       0.01f,
                       0.0f,
                       100.0f);
      ImGui::DragFloat("Bloom Threshold",
                       &gfx.bloomThreshold,
                       0.01f,
                       0.0f,
                       FLT_MAX);
      ImGui::InputInt("Bloom Iteration Count", &gfx.bloomIterationCount, 1, 2);
    }
  } // namespace Editor
} // namespace ToolKit