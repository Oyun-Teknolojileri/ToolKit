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
      m_name               = "Plugin";
      m_settings           = &g_app->m_simulatorSettings;
      m_numDefaultResNames = (int) m_emulatorResolutionNames.size();
    }

    PluginWindow::PluginWindow(XmlNode* node) : PluginWindow()
    {
      DeSerialize(nullptr, node);
    }

    PluginWindow::~PluginWindow() {}

    void PluginWindow::AddResolutionName(const String& name)
    {
      m_emulatorResolutionNames.push_back(name);
      m_screenResolutions.push_back(Vec2(500.0f, 500.0f));
    }

    void PluginWindow::RemoveResolutionName(int index)
    {
      bool canRemove = index > 0 || index < m_screenResolutions.size();

      assert(canRemove && "resolution index invalid");

      if (canRemove)
      {
        m_screenResolutions.erase(m_screenResolutions.begin() + index);
        m_emulatorResolutionNames.erase(m_emulatorResolutionNames.begin() +
                                        index);
      }
    }

    void PluginWindow::RemoveResolutionName(const String& name)
    {
      for (int i = 0; i < m_emulatorResolutionNames.size(); ++i)
      {
        if (name == m_emulatorResolutionNames[i])
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
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, g_blueTintButtonHoverColor);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, g_blueTintButtonActiveColor);
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
      float offset   = glm::max(ImGui::GetWindowWidth() * 0.5f - 100.0f, 0.0f);
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
      return m_emulatorResolutionNames[(uint) emuRes];
    }

    void PluginWindow::ShowSettings()
    {
      if (!m_settings->Windowed)
      {
        return;
      }

      // Resolution Bar
      EmulatorResolution resolution = m_settings->Resolution;
      int resolutionType            = (int) resolution;
      resolutionType =
          glm::min(resolutionType, (int) m_screenResolutions.size() - 1);

      Vec2 textSize =
          ImGui::CalcTextSize(m_emulatorResolutionNames[resolutionType].data());
      ImGui::SetNextItemWidth(textSize.x * 1.3f);

      AddResolutionName("Edit Resolutions");

      int lastEnumIndex = (int) m_emulatorResolutionNames.size() - 1;

      // in order to send to imgui we should convert to ptr array
      std::vector<char*> enumNames;
      for (size_t i = 0ull; i <= lastEnumIndex; i++)
      {
        enumNames.push_back(&m_emulatorResolutionNames[i][0]);
      }

      if (ImGui::Combo("##Resolution",
                       &resolutionType,
                       enumNames.data(),
                       (int) enumNames.size()))
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

          IVec2 resolutionSize   = m_screenResolutions[resolutionType];

          m_settings->Width      = (float) resolutionSize.x;
          m_settings->Height     = (float) resolutionSize.y;

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

        for (int i = m_numDefaultResNames;
             i < (int) m_emulatorResolutionNames.size();
             i++)
        {
          ImGui::PushID(i * 333);
          ImGui::InputText("name", m_emulatorResolutionNames[i].data(), 32);
          ImGui::SameLine();
          ImGui::InputInt2("size", &m_screenResolutions[i].x);
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
          AddResolutionName("new resolution\0");
        }
        ImGui::SameLine();
        // show apply button to save resolution settings
        float cursorX = ImGui::GetCursorPosX();
        ImGui::SetCursorPosX(
            glm::max(cursorX + ImGui::GetWindowWidth() - 150.0f, 150.0f));

        bool hasCustomRes = m_screenResolutions.size() >= m_numDefaultResNames;
        if (hasCustomRes &&
            ImGui::Button("Apply")) // show apply button if has custom res
        {
          Vec2 resSize      = m_screenResolutions[(int) m_settings->Resolution];
          m_settings->Width = (float) resSize.x;
          m_settings->Height = (float) resSize.y;
          UpdateSimulationWndSize();
        }

        ImGui::SetCursorPosX(cursorX);

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
