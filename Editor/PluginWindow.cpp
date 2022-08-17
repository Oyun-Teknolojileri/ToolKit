#include "PluginWindow.h"
#include "GlobalDef.h"
#include "ConsoleWindow.h"
#include "App.h"
#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {
    PluginWindow::PluginWindow()
    {
      m_name = "Plugin";
    }

    PluginWindow::PluginWindow(XmlNode* node)
    {
      DeSerialize(nullptr, node);
    }

    PluginWindow::~PluginWindow()
    {
    }

    void PluginWindow::Show()
    {
      ImGui::SetNextWindowSize(ImVec2(350, 150), ImGuiCond_Once);
      if
      (
        ImGui::Begin
        (
          "Simulation",
          &m_visible,
          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
        )
      )
      {
        HandleStates();

        ShowHeader();
        ImGui::Separator();

        ShowSimButtons();
        ImGui::Separator();

        ShowSettings();
      }
      ImGui::End();
    }

    Window::Type PluginWindow::GetType() const
    {
      return Type::PluginWindow;
    }

    void PluginWindow::Serialize(XmlDocument* doc, XmlNode* parent) const
    {
      Window::Serialize(doc, parent);
    }

    void PluginWindow::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      Window::DeSerialize(doc, parent);
    }

    void PluginWindow::ShowHeader()
    {
      int playwidth = static_cast<int>
        (g_app->m_emulatorSettings.playWidth);
      int playHeight = static_cast<int>
        (g_app->m_emulatorSettings.playHeight);

      String preset =
      EmuResToString(g_app->m_emulatorSettings.emuRes) +
      " / " +
      std::to_string(playwidth) +
      "x" +
      std::to_string(playHeight);

      String section = "Device: " + preset;
      ImGui::Text(section.c_str());

      if (m_simulationModeDisabled)
      {
        ImGui::BeginDisabled(m_simulationModeDisabled);
      }
      ImGui::Checkbox("Run In Window", &g_app->m_emulatorSettings.runWindowed);
      if (m_simulationModeDisabled)
      {
        ImGui::EndDisabled();
      }
    }

    void PluginWindow::ShowSimButtons()
    {
      Vec2 min = ImGui::GetWindowContentRegionMin();
      Vec2 max = ImGui::GetWindowContentRegionMax();
      Vec2 size = max - min;

      // Draw play - pause - stop buttons.
      float btnWidth = 24.0f;
      float offset = (size.x - btnWidth * 4.0f) * 0.5f;

      ImGui::SameLine(offset);
      float curYoff = ImGui::GetCursorPosY() + 10.0f;
      ImGui::SetCursorPosY(curYoff);

      if (g_app->m_gameMod == GameMod::Playing)
      {
        // Blue tint.
        ImGui::PushStyleColor
        (
          ImGuiCol_Button,
          (ImVec4)ImColor::HSV(4 / 7.0f, 0.6f, 0.6f)
        );
        ImGui::PushStyleColor
        (
          ImGuiCol_ButtonHovered,
          (ImVec4)ImColor::HSV(4 / 7.0f, 0.7f, 0.7f)
        );
        ImGui::PushStyleColor
        (
          ImGuiCol_ButtonActive,
          (ImVec4)ImColor::HSV(4 / 7.0f, 0.8f, 0.8f)
        );

        // Pause.
        if
        (
          ImGui::ImageButton
          (
            Convert2ImGuiTexture(UI::m_pauseIcon),
            ImVec2(btnWidth, btnWidth)
          )
        )
        {
          g_app->SetGameMod(GameMod::Paused);
        }

        ImGui::PopStyleColor(3);
      }
      else
      {
        // Green tint.
        ImGui::PushStyleColor
        (
          ImGuiCol_Button,
          (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f)
        );
        ImGui::PushStyleColor
        (
          ImGuiCol_ButtonHovered,
          (ImVec4)ImColor::HSV(2 / 7.0f, 0.7f, 0.7f)
        );
        ImGui::PushStyleColor
        (
          ImGuiCol_ButtonActive,
          (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.8f)
        );

        // Play.
        if
        (
          ImGui::ImageButton
          (
            Convert2ImGuiTexture(UI::m_playIcon),
            ImVec2(btnWidth, btnWidth)
          )
        )
        {
          m_simulationModeDisabled = true;
          g_app->SetGameMod(GameMod::Playing);
        }

        ImGui::PopStyleColor(3);
      }

      ImGui::SameLine();

      // Red tint.
      ImGui::PushStyleColor
      (
        ImGuiCol_Button,
        (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f)
      );
      ImGui::PushStyleColor
      (
        ImGuiCol_ButtonHovered,
        (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.7f)
      );
      ImGui::PushStyleColor
      (
        ImGuiCol_ButtonActive,
        (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.8f)
      );

      // Stop.
      if
      (
        ImGui::ImageButton
        (
          Convert2ImGuiTexture(UI::m_stopIcon),
          ImVec2(btnWidth, btnWidth)
        )
      )
      {
        if (g_app->m_gameMod != GameMod::Stop)
        {
          m_simulationModeDisabled = false;
          g_app->SetGameMod(GameMod::Stop);
        }
      }

      ImGui::PopStyleColor(3);
      ImGui::SameLine();

      if
      (
        ImGui::ImageButton
        (
          Convert2ImGuiTexture(UI::m_vsCodeIcon),
          ImVec2(btnWidth, btnWidth)
        )
      )
      {
        String codePath = g_app->m_workspace.GetCodePath();
        if (CheckFile(codePath))
        {
          String cmd = "code \"" + codePath + "\"";
          int result = std::system(cmd.c_str());
          if (result != 0)
          {
            g_app->GetConsole()->AddLog
            (
              "Visual Studio Code can't be started. "
              "Make sure it is installed.",
              LogType::Error
            );
          }
        }
        else
        {
          g_app->GetConsole()->AddLog
          (
            "There is not a vaild code folder.",
            LogType::Error
          );
        }
      }
    }

    void PluginWindow::ShowSettings()
    {
      // Emulator Settings
      ImVec2 settingsRegion = ImVec2
      (
        ImGui::GetWindowWidth(),
        ImGui::GetWindowHeight() - ImGui::GetCursorPosY()
      );

      ImGui::BeginChild
      (
        "##emuSettings",
        settingsRegion,
        false,
        ImGuiWindowFlags_AlwaysVerticalScrollbar
      );

      if (ImGui::BeginTable("EmuSet1", 2, ImGuiTableFlags_SizingFixedFit))
      {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);

        // Resolution Bar
        EmulatorResolution resolution = g_app->m_emulatorSettings.emuRes;

        int resolutionType = static_cast<int>(resolution);

        ImGui::Text("Resolution");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(150.0f);
        if
        (
          ImGui::Combo
          (
            "##dropdown",
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
            "Galaxy A51 / A71"
          )
        )
        {
          EmulatorResolution resolution =
            static_cast<EmulatorResolution> (resolutionType);
          switch (resolution)
          {
            case EmulatorResolution::Iphone_SE:
              g_app->m_emulatorSettings.playWidth = 375;
              g_app->m_emulatorSettings.playHeight = 667;
            break;
            case EmulatorResolution::Iphone_XR:
              g_app->m_emulatorSettings.playWidth = 414;
              g_app->m_emulatorSettings.playHeight = 896;
            break;
            case EmulatorResolution::Iphone_12_Pro:
              g_app->m_emulatorSettings.playWidth = 390;
              g_app->m_emulatorSettings.playHeight = 844;
            break;
            case EmulatorResolution::Pixel_5:
              g_app->m_emulatorSettings.playWidth = 393;
              g_app->m_emulatorSettings.playHeight = 851;
            break;
            case EmulatorResolution::Galaxy_S20_Ultra:
              g_app->m_emulatorSettings.playWidth = 412;
              g_app->m_emulatorSettings.playHeight = 915;
            break;
            case EmulatorResolution::Galaxy_Note20:
              g_app->m_emulatorSettings.playWidth = 412;
              g_app->m_emulatorSettings.playHeight = 915;
            break;
            case EmulatorResolution::Galaxy_Note20_Ultra:
              g_app->m_emulatorSettings.playWidth = 390;
              g_app->m_emulatorSettings.playHeight = 844;
            break;
            case EmulatorResolution::Ipad_Air:
              g_app->m_emulatorSettings.playWidth = 820;
              g_app->m_emulatorSettings.playHeight = 1180;
            break;
            case EmulatorResolution::Ipad_Mini:
              g_app->m_emulatorSettings.playWidth = 768;
              g_app->m_emulatorSettings.playHeight = 1024;
            break;
            case EmulatorResolution::Surface_Pro_7:
              g_app->m_emulatorSettings.playWidth = 912;
              g_app->m_emulatorSettings.playHeight = 1398;
            break;
            case EmulatorResolution::Surface_Duo:
              g_app->m_emulatorSettings.playWidth = 540;
              g_app->m_emulatorSettings.playHeight = 720;
            break;
            case EmulatorResolution::Galaxy_A51_A71:
              g_app->m_emulatorSettings.playWidth = 412;
              g_app->m_emulatorSettings.playHeight = 914;
            break;
          }

          g_app->m_emulatorSettings.emuRes = resolution;
          g_app->m_windowCamLoad = true;
        }

        // Width - Height
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        bool isCustomSized =
          g_app->m_emulatorSettings.emuRes == EmulatorResolution::Custom;
        if (!isCustomSized)
        {
          ImGui::BeginDisabled();
        }

        ImGui::Text("Width");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(150.0f);

        ImGui::DragFloat
        (
          "##w",
          &g_app->m_emulatorSettings.playWidth,
          1.0f,
          1.0f,
          4096.0f,
          "%.0f"
        );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Height");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(150.0f);

        ImGui::DragFloat
        (
          "##h",
          &g_app->m_emulatorSettings.playHeight,
          1.0f,
          1.0f,
          4096.0f,
          "%.0f"
        );

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

        ImGui::SliderFloat
        (
          "##z",
          &g_app->m_emulatorSettings.zoomAmount,
          0.25f,
          1.0f,
          "x%.2f"
        );

        // Landscape - Portrait Toggle
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Rotate");
        ImGui::TableSetColumnIndex(1);

        if
        (
          ImGui::ImageButton
          (
            Convert2ImGuiTexture(UI::m_phoneRotateIcon),
            ImVec2(30, 30)
          )
        )
        {
          bool* rotated = &g_app->m_emulatorSettings.landscape;
          *rotated = !(*rotated);
        }
        ImGui::EndTable();
      }

      ImGui::EndChild();
    }

    String PluginWindow::EmuResToString(EmulatorResolution emuRes)
    {
      switch (emuRes)
      {
        case EmulatorResolution::Custom :
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

  }  // namespace Editor

}  // namespace ToolKit
