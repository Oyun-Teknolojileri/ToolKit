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
      if (ImGui::Begin("Simulation##Plgn",
                       &m_visible,
                       ImGuiWindowFlags_NoScrollbar |
                           ImGuiWindowFlags_NoScrollWithMouse))
      {
        HandleStates();

        ShowHeader();
        ImGui::Separator();

        ShowActionButtons();
        ImGui::Separator();

        ShowSettings();
      }
      ImGui::End();
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
      String preset = EmuResToString(m_settings->Resolution) + " / " +
                      std::to_string(static_cast<int>(m_settings->Width)) +
                      "x" +
                      std::to_string(static_cast<int>(m_settings->Height));

      String section = "Device: " + preset;
      ImGui::Text(section.c_str());

      if (m_simulationModeDisabled)
      {
        ImGui::BeginDisabled(m_simulationModeDisabled);
      }
      ImGui::Checkbox("Run In Window", &m_settings->Windowed);
      if (m_simulationModeDisabled)
      {
        ImGui::EndDisabled();
      }
    }

    void PluginWindow::ShowActionButtons()
    {
      Vec2 min       = ImGui::GetWindowContentRegionMin();
      Vec2 max       = ImGui::GetWindowContentRegionMax();
      Vec2 size      = max - min;

      // Draw play - pause - stop buttons.
      float btnWidth = 24.0f;
      float offset   = (size.x - btnWidth * 5.0f) * 0.5f;

      float curYoff = ImGui::GetCursorPosY() + 10.0f;
      ImGui::SameLine(offset);
      ImGui::SetCursorPosY(curYoff);

      if (g_app->m_gameMod == GameMod::Playing)
      {
        // Blue tint.
        ImGui::PushStyleColor(ImGuiCol_Button, g_blueTintButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              g_blueTintButtonHoverColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              g_blueTintButtonActiveColor);

        // Pause.
        if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_pauseIcon),
                               ImVec2(btnWidth, btnWidth)))
        {
          g_app->SetGameMod(GameMod::Paused);
        }

        ImGui::PopStyleColor(3);
      }
      else
      {
        // Green tint.
        ImGui::PushStyleColor(ImGuiCol_Button, g_greenTintButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              g_greenTintButtonHoverColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              g_greenTintButtonActiveColor);

        // Play.
        if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_playIcon),
                               ImVec2(btnWidth, btnWidth)))
        {
          m_simulationModeDisabled = true;
          g_app->SetGameMod(GameMod::Playing);
        }

        ImGui::PopStyleColor(3);
      }

      // Red tint.
      ImGui::PushStyleColor(ImGuiCol_Button, g_redTintButtonColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, g_redTintButtonHoverColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, g_redTintButtonActiveColor);

      // Stop.
      ImGui::SameLine();
      ImGui::SetCursorPosY(curYoff);

      if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_stopIcon),
                             ImVec2(btnWidth, btnWidth)))
      {
        if (g_app->m_gameMod != GameMod::Stop)
        {
          m_simulationModeDisabled = false;
          g_app->SetGameMod(GameMod::Stop);
        }
      }

      ImGui::PopStyleColor(3);
      ImGui::SameLine();
      ImGui::SetCursorPosY(curYoff);

      // VS Code.
      if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_vsCodeIcon),
                             ImVec2(btnWidth, btnWidth)))
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
      ImGui::SetCursorPosY(curYoff);

      if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_buildIcn),
                             ImVec2(btnWidth, btnWidth)))
      {
        g_app->CompilePlugin();
      }

      UI::HelpMarker(TKLoc, "Build\nBuilds the projects code files.");
    }

    void PluginWindow::ShowSettings()
    {
      // Emulator Settings
      ImVec2 settingsRegion =
          ImVec2(ImGui::GetWindowWidth(),
                 ImGui::GetWindowHeight() - ImGui::GetCursorPosY());

      ImGui::BeginChild("##emuSettings",
                        settingsRegion,
                        false,
                        ImGuiWindowFlags_AlwaysVerticalScrollbar);

      if (ImGui::BeginTable("EmuSet1", 2, ImGuiTableFlags_SizingFixedFit))
      {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);

        // Resolution Bar
        EmulatorResolution resolution = m_settings->Resolution;

        int resolutionType            = static_cast<int>(resolution);

        ImGui::Text("Resolution");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(150.0f);
        if (ImGui::Combo("##dropdown",
                         &resolutionType,
                         "Custom\0"
                         "iPhone SE\0"
                         "iPhone XR\0"
                         "iPhone 12 Pro\0"
                         "Pixel 5\0"
                         "Galaxy S20 Ultra\0"
                         "Galaxy Note 20\0"
                         "Galaxy Note 20 Ultra\0"
                         "Ipad Air\0"
                         "Ipad Mini\0"
                         "Surface Pro 7\0"
                         "Surface Duo\0"
                         "Galaxy A51 / A71"))
        {
          EmulatorResolution resolution =
              static_cast<EmulatorResolution>(resolutionType);

          switch (resolution)
          {
          case EmulatorResolution::Iphone_SE:
            m_settings->Width  = 375;
            m_settings->Height = 667;
            break;
          case EmulatorResolution::Iphone_XR:
            m_settings->Width  = 414;
            m_settings->Height = 896;
            break;
          case EmulatorResolution::Iphone_12_Pro:
            m_settings->Width  = 390;
            m_settings->Height = 844;
            break;
          case EmulatorResolution::Pixel_5:
            m_settings->Width  = 393;
            m_settings->Height = 851;
            break;
          case EmulatorResolution::Galaxy_S20_Ultra:
            m_settings->Width  = 412;
            m_settings->Height = 915;
            break;
          case EmulatorResolution::Galaxy_Note20:
            m_settings->Width  = 412;
            m_settings->Height = 915;
            break;
          case EmulatorResolution::Galaxy_Note20_Ultra:
            m_settings->Width  = 390;
            m_settings->Height = 844;
            break;
          case EmulatorResolution::Ipad_Air:
            m_settings->Width  = 820;
            m_settings->Height = 1180;
            break;
          case EmulatorResolution::Ipad_Mini:
            m_settings->Width  = 768;
            m_settings->Height = 1024;
            break;
          case EmulatorResolution::Surface_Pro_7:
            m_settings->Width  = 912;
            m_settings->Height = 1398;
            break;
          case EmulatorResolution::Surface_Duo:
            m_settings->Width  = 540;
            m_settings->Height = 720;
            break;
          case EmulatorResolution::Galaxy_A51_A71:
            m_settings->Width  = 412;
            m_settings->Height = 914;
            break;
          }

          m_settings->Resolution = resolution;
          g_app->m_windowCamLoad = true;
          UpdateSimulationWndSize();
        }

        // Width - Height
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        bool isCustomSized =
            m_settings->Resolution == EmulatorResolution::Custom;

        if (!isCustomSized)
        {
          ImGui::BeginDisabled();
        }

        ImGui::Text("Width");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(150.0f);

        if (ImGui::DragFloat("##w",
                             &m_settings->Width,
                             1.0f,
                             1.0f,
                             4096.0f,
                             "%.0f"))
        {
          UpdateSimulationWndSize();
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Height");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(150.0f);

        if (ImGui::DragFloat("##h",
                             &m_settings->Height,
                             1.0f,
                             1.0f,
                             4096.0f,
                             "%.0f"))
        {
          UpdateSimulationWndSize();
        }

        if (!isCustomSized)
        {
          ImGui::EndDisabled();
        }

        // Zoom
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Zoom");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(150.0f);

        if (ImGui::SliderFloat("##z", &m_settings->Scale, 0.25f, 1.0f, "x%.2f"))
        {
          UpdateSimulationWndSize();
        }

        // Landscape - Portrait Toggle
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Rotate");
        ImGui::TableSetColumnIndex(1);

        if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_phoneRotateIcon),
                               ImVec2(30, 30)))
        {
          m_settings->Landscape = !m_settings->Landscape;
          UpdateSimulationWndSize();
        }
        ImGui::EndTable();
      }

      ImGui::EndChild();
    }

    String PluginWindow::EmuResToString(EmulatorResolution emuRes)
    {
      switch (emuRes)
      {
      case EmulatorResolution::Custom:
        return "Custom";
        break;
      case EmulatorResolution::Galaxy_A51_A71:
        return "Galaxy A51 / 71";
        break;
      case EmulatorResolution::Galaxy_Note20:
        return "Galaxy Note 20";
        break;
      case EmulatorResolution::Galaxy_Note20_Ultra:
        return "Galaxy Note 20 Ultra";
        break;
      case EmulatorResolution::Galaxy_S20_Ultra:
        return "Galaxy S20 Ultra";
        break;
      case EmulatorResolution::Ipad_Air:
        return "Ipad Air";
        break;
      case EmulatorResolution::Ipad_Mini:
        return "Ipad Mini";
        break;
      case EmulatorResolution::Iphone_12_Pro:
        return "Iphone 12 Pro";
        break;
      case EmulatorResolution::Iphone_SE:
        return "Iphone SE";
        break;
      case EmulatorResolution::Iphone_XR:
        return "Iphone XR";
        break;
      case EmulatorResolution::Pixel_5:
        return "Pixel 5";
        break;
      case EmulatorResolution::Surface_Duo:
        return "Surface Duo";
        break;
      case EmulatorResolution::Surface_Pro_7:
        return "Surface Pro 7";
        break;
      default:
        assert(false && "Resolution not found.");
        return "Err";
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
