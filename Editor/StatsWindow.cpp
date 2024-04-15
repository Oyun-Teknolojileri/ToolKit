/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "StatsWindow.h"

#include "EditorTypes.h"

#include <TKStats.h>

namespace ToolKit
{
  namespace Editor
  {

    TKDefineClass(StatsWindow, Window);

    StatsWindow::StatsWindow() { m_name = g_statsView; }

    StatsWindow::~StatsWindow() {}

    void StatsWindow::Show()
    {
      TKStats* tkStats = GetTKStats();
      if (tkStats == nullptr)
      {
        return;
      }

      ImGui::SetNextWindowSize(ImVec2(270, 110), ImGuiCond_Once);
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        float cpuTime, gpuTime;
        GetRenderTime(cpuTime, gpuTime);

        bool* gpuTimer             = &GetEngineSettings().Graphics.enableGpuTimer;
        EditorViewportPtr viewport = g_app->GetViewport(g_3dViewport);

        ImGui::Checkbox("##GpuProfileOn", gpuTimer);
        UI::AddTooltipToLastItem("Have a negative impact on cpu performance.\nEnable it to see the gpu times.");
        ImGui::SameLine();
        ImGui::Text("Viewport Resolution: %dx%d",
                    (int) viewport->m_wndContentAreaSize.x,
                    (int) viewport->m_wndContentAreaSize.y);
        ImGui::Text("Render Time (gpu-ms): %.2f, FPS: %.2f", gpuTime, 1000.0f / gpuTime);
        ImGui::Text("Render Time (cpu-ms): %.2f, FPS: %.2f", cpuTime, 1000.0f / cpuTime);
        ImGui::Text("Editor FPS: %d", g_app->m_fps);
        ImGui::Spacing();
        ImGui::Text("Total Draw Call: %llu", g_app->GetLastFrameDrawCallCount());
        ImGui::Text("Total Hardware Render Pass: %llu", g_app->GetLastFrameHWRenderPassCount());
        ImGui::Text("Approximate Total VRAM Usage: %llu MB", GetTotalVRAMUsageInMB());
      }
      ImGui::End();
    }

  } // namespace Editor
} // namespace ToolKit
