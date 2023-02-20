#include "MultiChoiceParameterWindow.h"

#include "App.h"
#include "ImGui/imgui_stdlib.h"
#include "stdafx.h"

namespace ToolKit::Editor
{
  bool MultiChoiceParameterWindow::IsVariantValid()
  {
    if (m_variant.Choices.size() < 2ull)
    {
      g_app->m_statusMsg = "Failed!";
      GetLogger()->WriteConsole(LogType::Warning,
                                "You must define at least two parameter!");
      return false;
    }
    return true;
  }

  void MultiChoiceParameterWindow::ShowVariant()
  {
    ImGui::Text("New Variant");
    // draw&edit each parameter
    for (size_t i = 0; i < m_variant.Choices.size(); ++i)
    {
      ParameterVariant* var = &m_variant.Choices[i].second;
      String& name          = m_variant.Choices[i].first;

      ImGui::PushID(i * 2000 + 1);
      ImGui::InputText("", &name);
      ImGui::PopID();

      ImGui::PushID(i * 2000 + 2);

      switch (var->GetType())
      {
      case ParameterVariant::VariantType::String:
      {
        ImGui::InputText("", var->GetVarPtr<String>());
      }
      break;
      case ParameterVariant::VariantType::Bool:
      {
        bool val = var->GetVar<bool>();
        if (ImGui::Checkbox("", &val))
        {
          *var = val;
        }
      }
      break;
      case ParameterVariant::VariantType::Int:
      {
        ImGui::InputInt("", var->GetVarPtr<int>());
      }
      break;
      case ParameterVariant::VariantType::Float:
      {
        ImGui::DragFloat("", var->GetVarPtr<float>(), 0.1f);
      }
      break;
      case ParameterVariant::VariantType::Vec3:
      {
        ImGui::DragFloat3("", &var->GetVar<Vec3>()[0], 0.1f);
      }
      break;
      case ParameterVariant::VariantType::Vec4:
      {
        ImGui::DragFloat4("", &var->GetVar<Vec4>()[0], 0.1f);
      }
      break;
      case ParameterVariant::VariantType::Mat3:
      {
        Vec3 vec;
        Mat3 val = var->GetVar<Mat3>();
        for (int j = 0; j < 3; j++)
        {
          vec = glm::row(val, j);
          ImGui::InputFloat3("", &vec[0]);
          val  = glm::row(val, j, vec);
          *var = val;
        }
      }
      break;
      case ParameterVariant::VariantType::Mat4:
      {
        Vec4 vec;
        Mat4 val = var->GetVar<Mat4>();
        for (int j = 0; j < 4; j++)
        {
          vec = glm::row(val, j);
          ImGui::InputFloat4("", &vec[0]);
          val  = glm::row(val, j, vec);
          *var = val;
        }
      }
      break;
      }
      // align X button to the most right of the window
      ImGui::SameLine(ImGui::GetWindowWidth() - 30);

      if (ImGui::Button("X"))
      {
        auto& choices = m_variant.Choices;
        m_variant.Choices.erase(choices.begin() + i);
        i--;
      }
      ImGui::PopID();
      ImGui::Spacing();
    }

    int dataType = 0;
    if (ImGui::Combo(
            "AddChoice",
            &dataType,
            "Sellect Type"
            "\0String\0Boolean\0Int\0Float\0Vec2\0Vec3\0Vec4\0Mat3\0Mat4"))
    {
      switch (dataType)
      {
      case 0:
      case 1:
        m_variant.Choices.push_back(
            std::make_pair(String("name"), ParameterVariant(String(""))));
        break;
      case 2:
        m_variant.Choices.push_back(
            std::make_pair(String("name"), ParameterVariant(false)));
        break;
      case 3:
        m_variant.Choices.push_back(
            std::make_pair(String("name"), ParameterVariant(0)));
        break;
      case 4:
        m_variant.Choices.push_back(
            std::make_pair(String("name"), ParameterVariant(0.0f)));
        break;
      case 5:
        m_variant.Choices.push_back(
            std::make_pair(String("name"), ParameterVariant(Vec2())));
        break;
      case 6:
        m_variant.Choices.push_back(
            std::make_pair(String("name"), ParameterVariant(Vec3())));
        break;
      case 7:
        m_variant.Choices.push_back(
            std::make_pair(String("name"), ParameterVariant(Vec4())));
        break;
      case 8:
        m_variant.Choices.push_back(
            std::make_pair(String("name"), ParameterVariant(Mat3())));
        break;
      case 9:
        m_variant.Choices.push_back(
            std::make_pair(String("name"), ParameterVariant(Mat4())));
        break;
      default:
        assert(false && "parameter type invalid");
        break;
      }
    }
  }

  void MultiChoiceParameterWindow::Show()
  {
    ImGuiIO io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_Once);
    ImGui::SetNextWindowPos(
        ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
        ImGuiCond_Once,
        ImVec2(0.5f, 0.5f));

    ImGui::Begin("MultiChoice Parameter Create Window", nullptr);

    ShowVariant();

    if (ImGui::Button("Create"))
    {
      if (!IsVariantValid()) 
      {
        ImGui::End();
        return;
      }
      
      ParameterVariant customVar;
      // This makes them only visible in Custom Data dropdown.
      customVar.m_exposed  = true;
      customVar.m_editable = true;
      customVar.m_category = CustomDataCategory;
      customVar            = m_variant;

      m_parameter->Add(customVar);
      m_menuOpen = false;
      UI::RemoveTempWindow(this);
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
      m_menuOpen = false;
      UI::RemoveTempWindow(this);
    }

    ImGui::End();
  }

  void MultiChoiceParameterWindow::OpenCreateWindow(ParameterBlock* parameter)
  {
    if (m_menuOpen)
      return;

    UI::AddTempWindow(this);
    m_variant = MultiChoiceVariant();
    m_menuOpen = true;
    m_parameter = parameter;
  }

  MultiChoiceParameterWindow* MultiChoiceParameterWindow::Instance()
  {
    static std::unique_ptr<MultiChoiceParameterWindow> instance =
        std::make_unique<MultiChoiceParameterWindow>();
    return instance.get();
  }
} // namespace ToolKit::Editor
