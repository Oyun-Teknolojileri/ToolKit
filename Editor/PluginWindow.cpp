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
    const int MaxResolutionNameCnt = 24;
    const int MaxResolutionNameWidth = 32;

    static char EmulatorResolutionNames[MaxResolutionNameCnt]
                                       [MaxResolutionNameWidth]
    {
      "Custom Resolutions\0",
      "Full HD (1080p)\0",
      "QHD (1440p)\0",
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

    static IVec2 ScreenResolutions[MaxResolutionNameCnt] =
    {
      IVec2(480, 667), // default
      IVec2(375, 667), // Iphone_SE,
      IVec2(414, 896), // Iphone_XR,
      IVec2(390, 844), // Iphone_12_Pro,
      IVec2(393, 851), // Pixel_5,
      IVec2(412, 915), // Galaxy_S20_Ultra,
      IVec2(412, 915), // Galaxy_Note20,
      IVec2(390, 844), // Galaxy_Note20_Ultra,
      IVec2(820, 118), // Ipad_Air,
      IVec2(768, 102), // Ipad_Mini,
      IVec2(912, 139), // Surface_Pro_7,
      IVec2(540, 720), // Surface_Duo,
      IVec2(412, 914)  // Galaxy_A51_A71
    };

    PluginWindow::PluginWindow()
    {
      m_name     = "Plugin";
      m_settings = &g_app->m_simulatorSettings;
      m_numEmulatorResNames = 0;
      // calculate number of default resolution names,
      // greater than 31 means ascii alpha numeric
      while (EmulatorResolutionNames[m_numEmulatorResNames][0] >= 31) 
      {
        m_numEmulatorResNames++;
      }
      
      m_numDefaultResNames = m_numEmulatorResNames;
      
      // fill remaining array with valid resolutions
      for (int i = m_numDefaultResNames; i < MaxResolutionNameCnt; ++i)
      {
        ScreenResolutions[i].x = ScreenResolutions[i].y = 500.0f;
      }
    }

    PluginWindow::PluginWindow(XmlNode* node) : PluginWindow()
    {
      DeSerialize(nullptr, node);
    }

    PluginWindow::~PluginWindow() {}

    void PluginWindow::AddResolutionName(const char* name)
    {
      assert(m_numEmulatorResNames < MaxResolutionNameCnt);
      int len = 0;
      while (*name)
      {
        EmulatorResolutionNames[m_numEmulatorResNames][len++] = *name++;
      }
      // null terminate
      EmulatorResolutionNames[m_numEmulatorResNames++][len] = '\0'; 
    }

    void PluginWindow::RemoveResolutionName(int index)
    {
      assert(index < 0 || index < m_numEmulatorResNames 
             && "resolution index invalid");
      // remove this index
      int len = 0;
      while (EmulatorResolutionNames[index][len] != '\0')
      {
        EmulatorResolutionNames[index][len++] = '\0';
      }
      len = 0;
      int lastIdx = --m_numEmulatorResNames;
      // add last name to removed index
      while (EmulatorResolutionNames[lastIdx][len] != '\0')
      {
        char c = EmulatorResolutionNames[lastIdx][len];
        EmulatorResolutionNames[lastIdx][len] = '\0';
        EmulatorResolutionNames[index][len++] = c;
      }
    }

    void PluginWindow::RemoveResolutionName(const char* name)
    {
      for (int i = 0; i < m_numEmulatorResNames; ++i)
      {
        if (strcmp(name, EmulatorResolutionNames[i]) == 0)
        {
          RemoveResolutionName(i);
          return;
        }
      }
      GetLogger()->WriteConsole(LogType::Warning, 
                                "resolution name is not exist!");
    }

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

      if (ImGui::Button(ICON_FA_SLIDERS, ImVec2(26.0f, 26.0f)))
      {
        m_settings->Windowed = !m_settings->Windowed;
      }
      
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
      // pick middle point of the window and 
      // move left half of the width of action buttons(250.0f)
      float offset = glm::max(ImGui::GetWindowWidth() * 0.5f - 100.0f, 0.0f);
      ImGui::SetCursorPosX(offset);

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
        if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_playIcon), btnSize) &&
            !g_app->IsCompiling())
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

      ImGui::SetNextItemWidth(
          ImGui::CalcTextSize(EmulatorResolutionNames[resolutionType]).x *
          1.3f);

      AddResolutionName("Edit Resolutions");

      int lastEnumIndex = m_numEmulatorResNames - 1;
      // in order to send to imgui we should convert to ptr array
      const char* enumNames[MaxResolutionNameCnt];
      for (int i = 0; i <= lastEnumIndex; i++) 
      {
        enumNames[i] = EmulatorResolutionNames[i];
      }

      if (ImGui::Combo("##Resolution",
                       &resolutionType,
                       enumNames, 
                       m_numEmulatorResNames))
      {
        if (resolutionType == lastEnumIndex)
        {
          m_resolutionSettingsWindowEnabled = true;
          ImGui::SetNextWindowPos(ImGui::GetMousePos());
        }
        else
        {
          EmulatorResolution resolution =
            static_cast<EmulatorResolution>(resolutionType);
        
          IVec2 resolutionSize   = ScreenResolutions[resolutionType];

          m_settings->Width  = (float)resolutionSize.x;
          m_settings->Height = (float)resolutionSize.y;
      
          m_settings->Resolution = resolution;
          UpdateSimulationWndSize();
        }
      }
      RemoveResolutionName(lastEnumIndex);

      if (m_resolutionSettingsWindowEnabled)
      {
        ImGui::Begin("Edit Resolutions", 
                     &m_resolutionSettingsWindowEnabled, 
                     ImGuiWindowFlags_NoScrollWithMouse | 
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_AlwaysAutoResize);  
        
        for (int i = m_numDefaultResNames; i < m_numEmulatorResNames; ++i)
        {
          ImGui::PushID(i*333);
          ImGui::InputText("name",
                           EmulatorResolutionNames[i],
                           MaxResolutionNameWidth);
          ImGui::SameLine();  
          ImGui::InputInt2("size", &ScreenResolutions[i].x);
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_MINUS)) 
          {
            RemoveResolutionName(i);
          }
          ImGui::PopID();
        }
        
        ImGui::Text("Add New");
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLUS))
        {
          AddResolutionName("new resolution");
        }

        ImGui::End();
      }

      // Zoom
      ImGui::SameLine();
      ImGui::Text("Zoom");
      ImGui::SetNextItemWidth(60.0f);
      ImGui::SameLine();
      
      if (ImGui::DragFloat("##z", &m_settings->Scale, 0.05f, 0.0f, 1.0f))
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
