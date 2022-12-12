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
      int i = g_app->m_useAcesTonemapper;
      ImGui::InputInt("Use Aces Tonemapper", &i, 1, 1, 0);
      g_app->m_useAcesTonemapper = i;
    }
  } // namespace Editor
} // namespace ToolKit