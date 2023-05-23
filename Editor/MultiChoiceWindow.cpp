/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "MultiChoiceWindow.h"

#include "App.h"
#include "CustomDataView.h"

#include "DebugNew.h"

namespace ToolKit::Editor
{
  bool MultiChoiceCraeteWindow::IsVariantValid()
  {
    if (m_variant.Choices.size() < 2ull)
    {
      g_app->m_statusMsg = "Failed!";
      GetLogger()->WriteConsole(LogType::Warning, "You must define at least two parameter!");
      return false;
    }

    for (const ParameterVariant& var : m_variant.Choices)
    {
      if (var.m_name.size() < 1)
      {
        g_app->m_statusMsg = "Failed!";
        GetLogger()->WriteConsole(LogType::Warning, "name can't be empty!");
        return false;
      }
    }

    return true;
  }

  void MultiChoiceCraeteWindow::ShowVariant()
  {
    CustomDataView::BeginShowVariants("New Variant");
    ParameterVariant* remove = nullptr;
    // draw&edit each parameter
    for (size_t i = 0; i < m_variant.Choices.size(); ++i)
    {
      ParameterVariant* var = &m_variant.Choices[i];
      CustomDataView::ShowVariant(var, remove, i, true);
    }

    if (remove != nullptr)
    {
      auto find = std::find_if(m_variant.Choices.begin(),
                               m_variant.Choices.end(),
                               [remove](const ParameterVariant& param) { return param.m_id == remove->m_id; });
      m_variant.Choices.erase(find);
    }

    CustomDataView::EndShowVariants();

    int dataType = 0;
    if (ImGui::Combo("AddChoice",
                     &dataType,
                     "Sellect Type"
                     "\0String\0Boolean\0Int\0Float\0Vec2\0Vec3\0Vec4\0Mat3\0Mat4"))
    {
      switch (dataType)
      {
      case 0:
      case 1:
        m_variant.Choices.push_back(ParameterVariant(String("")));
        break;
      case 2:
        m_variant.Choices.push_back(ParameterVariant(false));
        break;
      case 3:
        m_variant.Choices.push_back(ParameterVariant(0));
        break;
      case 4:
        m_variant.Choices.push_back(ParameterVariant(0.0f));
        break;
      case 5:
        m_variant.Choices.push_back(ParameterVariant(Vec2()));
        break;
      case 6:
        m_variant.Choices.push_back(ParameterVariant(Vec3()));
        break;
      case 7:
        m_variant.Choices.push_back(ParameterVariant(Vec4()));
        break;
      case 8:
        m_variant.Choices.push_back(ParameterVariant(Mat3()));
        break;
      case 9:
        m_variant.Choices.push_back(ParameterVariant(Mat4()));
        break;
      default:
        assert(false && "parameter type invalid");
        break;
      }
    }
  }

  void MultiChoiceCraeteWindow::Show()
  {
    ImGuiIO io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
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

  void MultiChoiceCraeteWindow::OpenCreateWindow(ParameterBlock* parameter)
  {
    if (m_menuOpen)
      return;

    UI::AddTempWindow(this);
    m_variant   = MultiChoiceVariant();
    m_menuOpen  = true;
    m_parameter = parameter;
  }
} // namespace ToolKit::Editor