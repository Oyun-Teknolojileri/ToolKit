#include "StatsView.h"

#include "Global.h"

#include <TKStats.h>

namespace ToolKit
{
  namespace Editor
  {
    StatsView::StatsView() { m_name = g_statsView; }

    Window::Type StatsView::GetType() const { return Type::Stats; }

    StatsView::~StatsView() {}

    void StatsView::Show()
    {
      TKStats* tkStats = GetTKStats();
      if (tkStats == nullptr)
      {
        return;
      }

      ImGui::SetNextWindowSize(ImVec2(270, 110), ImGuiCond_Once);
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        ImGui::Text("FPS: %d", g_app->m_fps);
        ImGui::Spacing();
        ImGui::Text("Total Draw Call: %llu", g_app->GetLastFrameDrawCallCount());
        ImGui::Text("Total Hardware Render Pass: %llu", g_app->GetLastFrameHWRenderPassCount());
        ImGui::Text("Approximate Total VRAM Usage: %llu MB", GetTotalVRAMUsageInMB());
      }
      ImGui::End();
    }
  } // namespace Editor
} // namespace ToolKit
