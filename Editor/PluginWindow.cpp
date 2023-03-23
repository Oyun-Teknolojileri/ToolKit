#include "PluginWindow.h"

#include "App.h"
#include "ConsoleWindow.h"
#include "EditorViewport2d.h"
#include "Global.h"

#include <utility>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    PluginWindow::PluginWindow()
    {
      m_name     = "Plugin";
      m_settings = &g_app->m_simulatorSettings;
    }

    PluginWindow::PluginWindow(XmlNode* node) : PluginWindow()
    {
      DeSerialize(nullptr, node);
    }

    PluginWindow::~PluginWindow() {}

    void PluginWindow::Show()
    {
      ImGui::SetNextWindowSize(ImVec2(350, 150), ImGuiCond_Once);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5, 5});
      
      if (ImGui::Begin("Simulation##Plgn",
                       &m_visible,
                       ImGuiWindowFlags_NoScrollbar |
                           ImGuiWindowFlags_NoScrollWithMouse))
      {
        HandleStates();
        ShowActionButtons();
        ShowHeader();
        ShowSettings();
      }
      ImGui::End();
      ImGui::PopStyleVar();
    }

    Window::Type PluginWindow::GetType() const { return Type::PluginWindow; }

    void PluginWindow::Serialize(XmlDocument* doc, XmlNode* parent) const
    {
      Window::Serialize(doc, parent);
    }

    void PluginWindow::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      Window::DeSerialize(doc, parent);
    }

    void PluginWindow::UpdateSimulationWndSize()
    {
      if (g_app->m_simulationWindow)
      {
        uint width  = uint(m_settings->Width * m_settings->Scale);
        uint height = uint(m_settings->Height * m_settings->Scale);
        if (m_settings->Landscape)
        {
          std::swap(width, height);
        }

        g_app->m_simulationWindow->ResizeWindow(width, height);
        UpdateCanvas(width, height);
      }
    }

    void PluginWindow::ShowHeader()
    {
      if (m_simulationModeDisabled)
      {
        ImGui::BeginDisabled(m_simulationModeDisabled);
      }
      ImGui::Checkbox("Run In Window", &m_settings->Windowed);
      if (m_simulationModeDisabled)
      {
        ImGui::EndDisabled();
      }
      ImGui::SameLine();
    }

    static void GreenTint()
    {
      ImGui::PushStyleColor(ImGuiCol_Button, g_blueTintButtonColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                            g_blueTintButtonHoverColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                            g_blueTintButtonActiveColor);
    }

    static void RedTint()
    {
      ImGui::PushStyleColor(ImGuiCol_Button, g_redTintButtonColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, g_redTintButtonHoverColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, g_redTintButtonActiveColor);      
    }

    void PluginWindow::ShowActionButtons()
    {
      // Draw play - pause - stop buttons.
      ImVec2 btnSize = ImVec2(20.0f, 20.0f);
      if (m_settings->Windowed == false) 
      {
        // pick middle point of the window and move left half of the width of action buttons(250.0f)
        float offset = glm::max(ImGui::GetWindowWidth() * 0.5f - 125.0f, 0.0f);
        ImGui::SetCursorPosX(offset);
      }
      
      if (g_app->m_gameMod == GameMod::Playing)
      {
        GreenTint();
        // Pause.
        if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_pauseIcon), btnSize))
        {
          g_app->SetGameMod(GameMod::Paused);
        }

        ImGui::PopStyleColor(3);
      }
      else
      {
        GreenTint();
        // Play.
        if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_playIcon), btnSize))
        {
          m_simulationModeDisabled = true;
          g_app->SetGameMod(GameMod::Playing);
        }

        ImGui::PopStyleColor(3);
      }
      // Stop.
      ImGui::SameLine();
      RedTint();

      if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_stopIcon), btnSize))
      {
        if (g_app->m_gameMod != GameMod::Stop)
        {
          m_simulationModeDisabled = false;
          g_app->SetGameMod(GameMod::Stop);
        }
      }

      ImGui::PopStyleColor(3);
      ImGui::SameLine();

      // VS Code.
      if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_vsCodeIcon), btnSize))
      {
        String codePath = g_app->m_workspace.GetCodePath();
        if (CheckFile(codePath))
        {
          String cmd = "code \"" + codePath + "\"";
          int result = g_app->ExecSysCommand(cmd, true, false);
          if (result != 0)
          {
            g_app->GetConsole()->AddLog("Visual Studio Code can't be started. "
                                        "Make sure it is installed.",
                                        LogType::Error);
          }
        }
        else
        {
          g_app->GetConsole()->AddLog("There is not a vaild code folder.",
                                      LogType::Error);
        }
      }

      // Build.
      ImGui::SameLine();

      if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_buildIcn), btnSize))
      {
        g_app->CompilePlugin();
      }

      UI::HelpMarker(TKLoc, "Build\nBuilds the projects code files.");
      ImGui::SameLine();
    }

    static const char* EmulatorResolutionNames[] =
    {
      "Custom\0"               ,
      "iPhone SE (375x667)\0",
      "iPhone XR (414x896)\0",
      "iPhone 12 Pro (390x844)\0",
      "Pixel 5 (393x851)\0",
      "Galaxy S20 Ultra (412x915)\0",
      "Galaxy Note 20 (412x915)\0",
      "Galaxy Note 20 Ultra (390x844)\0",
      "Ipad Air  (820x118)\0",
      "Ipad Mini (768x102)\0",
      "Surface Pro 7 (912x139)\0",
      "Surface Duo (540x720)\0",
      "Galaxy A51 / A71 (412x914)\0",
    };

    String PluginWindow::EmuResToString(EmulatorResolution emuRes)
    {
      return EmulatorResolutionNames[(uint) emuRes];
    }

    void PluginWindow::ShowSettings()
    {
      if (!m_settings->Windowed) return;
      // Resolution Bar
      EmulatorResolution resolution = m_settings->Resolution;
      int resolutionType            = static_cast<int>(resolution);
    
      static const ImVec2 screenResolutionsLUT[] =
      {
        ImVec2(480, 667), // default
        ImVec2(375, 667), // Iphone_SE,
        ImVec2(414, 896), // Iphone_XR,
        ImVec2(390, 844), // Iphone_12_Pro,
        ImVec2(393, 851), // Pixel_5,
        ImVec2(412, 915), // Galaxy_S20_Ultra,
        ImVec2(412, 915), // Galaxy_Note20,
        ImVec2(390, 844), // Galaxy_Note20_Ultra,
        ImVec2(820, 118), // Ipad_Air,
        ImVec2(768, 102), // Ipad_Mini,
        ImVec2(912, 139), // Surface_Pro_7,
        ImVec2(540, 720), // Surface_Duo,
        ImVec2(412, 914)  // Galaxy_A51_A71
      };

      ImGui::Text("| Resolution");
      ImGui::SetNextItemWidth(200.0f);
      ImGui::SameLine();
      if (ImGui::Combo("##dropdown", &resolutionType, EmulatorResolutionNames, 
        IM_ARRAYSIZE(EmulatorResolutionNames)))
      {
        EmulatorResolution resolution =
            static_cast<EmulatorResolution>(resolutionType);
        
        ImVec2 resolutionSize = screenResolutionsLUT[resolutionType];

        m_settings->Width  = resolutionSize.x;
        m_settings->Height = resolutionSize.y;
      
        m_settings->Resolution = resolution;
        g_app->m_windowCamLoad = true;
        UpdateSimulationWndSize();
      }
      
      // Width - Height
      bool isCustomSized =
          m_settings->Resolution == EmulatorResolution::Custom;
      
      if (isCustomSized)
      {
        ImGui::SameLine();
        ImGui::Text("Width");
        ImGui::SetNextItemWidth(70.0f);
        ImGui::SameLine();
        if (ImGui::DragFloat("##w", &m_settings->Width, 1.0f, 1.0f, 4096.0f, 
                            "%.0f"))
        {
          UpdateSimulationWndSize();
        }
      
        ImGui::SameLine();
        ImGui::Text("Height");
        ImGui::SetNextItemWidth(70.0f);
        ImGui::SameLine();
      
        if (ImGui::DragFloat("##h", &m_settings->Height, 1.0f, 1.0f, 4096.0f,
                             "%.0f"))
        {
          UpdateSimulationWndSize();
        }
      }
      // Zoom
      ImGui::SameLine();
      ImGui::Text("Zoom");
      ImGui::SetNextItemWidth(70.0f);
      ImGui::SameLine();
      
      if (ImGui::SliderFloat("##z", &m_settings->Scale, 0.25f, 1.0f, "x%.2f"))
      {
        UpdateSimulationWndSize();
      }
      
      // Landscape - Portrait Toggle
      ImGui::SameLine();
      ImGui::Text("Rotate");
      ImGui::SameLine();
      
      if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_phoneRotateIcon),
                             ImVec2(20, 20)))
      {
        m_settings->Landscape = !m_settings->Landscape;
        UpdateSimulationWndSize();
      }
    }

    void PluginWindow::UpdateCanvas(uint width, uint height)
    {
      EditorViewport2d* viewport =
          g_app->GetWindow<EditorViewport2d>(g_2dViewport);

      if (viewport != nullptr)
      {
        UILayerRawPtrArray layers;
        GetUIManager()->GetLayers(viewport->m_viewportId, layers);

        g_app->GetCurrentScene()->ClearSelection();
        for (UILayer* layer : layers)
        {
          layer->ResizeUI((float) width, (float) height);
        }
      }
    }

  } // namespace Editor
} // namespace ToolKit
