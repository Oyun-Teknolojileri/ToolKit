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

    RenderSettingsView::~RenderSettingsView()
    {
    }

    void RenderSettingsView::Show()
    {
      const char* items[] = {"Off", "Reinhard", "ACES"};
      uint itemCount      = sizeof(items) / sizeof(items[0]);
      if (ImGui::BeginCombo("Tonemapper mode",
                            items[g_app->m_useAcesTonemapper]))
      {
        for (uint itemIndx = 0; itemIndx < itemCount; itemIndx++)
        {
          bool isSelected      = false;
          const char* itemName = items[itemIndx];
          ImGui::Selectable(itemName, &isSelected);
          if (isSelected)
          {
            g_app->m_useAcesTonemapper = itemIndx;
          }
        }

        ImGui::EndCombo();
      }
    }
  } // namespace Editor
} // namespace ToolKit