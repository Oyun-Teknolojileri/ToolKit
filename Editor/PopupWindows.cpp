/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "PopupWindows.h"

#include "App.h"

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {

    // StringInputWindow
    //////////////////////////////////////////////////////////////////////////

    TKDefineClass(StringInputWindow, Window);

    StringInputWindow::StringInputWindow(const String& name, bool showCancel)
    {
      m_name       = name;
      m_showCancel = showCancel;
      UI::m_volatileWindows.push_back(this);
    }

    void StringInputWindow::Show()
    {
      if (!m_visible)
      {
        return;
      }

      ImGuiIO& io = ImGui::GetIO();
      ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                              ImGuiCond_Once,
                              ImVec2(0.5f, 0.5f));

      ImGui::OpenPopup(m_name.c_str());
      if (ImGui::BeginPopupModal(m_name.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
      {
        if (ImGui::IsWindowAppearing())
        {
          ImGui::SetKeyboardFocusHere();
        }

        ImGui::InputTextWithHint(
            m_inputLabel.c_str(),
            m_hint.c_str(),
            &m_inputVal,
            ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackCharFilter,
            [](ImGuiInputTextCallbackData* data) -> int
            { return (reinterpret_cast<StringInputWindow*>(data->UserData))->FilterChars(data); },
            reinterpret_cast<void*>(this));

        // Center buttons.
        ImGui::BeginTable("##FilterZoom", m_showCancel ? 4 : 3, ImGuiTableFlags_SizingFixedFit);

        ImGui::TableSetupColumn("##spaceL", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##ok");
        if (m_showCancel)
        {
          ImGui::TableSetupColumn("##cancel");
        }
        ImGui::TableSetupColumn("##spaceR", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
          m_visible = false;
          m_taskFn(m_inputVal);
          m_inputVal.clear();
          ImGui::CloseCurrentPopup();
        }

        if (m_showCancel)
        {
          ImGui::TableNextColumn();
          if (ImGui::Button("Cancel", ImVec2(120, 0)))
          {
            m_visible = false;
            m_inputVal.clear();
            ImGui::CloseCurrentPopup();
          }
        }

        ImGui::EndTable();
        ImGui::EndPopup();
      }
    }

    int StringInputWindow::FilterChars(ImGuiInputTextCallbackData* data)
    {
      if (contains(m_illegalChars, (char) data->EventChar))
      {
        g_app->m_statusMsg = "Invalid character.";
        return 1;
      }
      return 0;
    }

    // YesNoWindow
    //////////////////////////////////////////////////////////////////////////

    TKDefineClass(YesNoWindow, Window);

    YesNoWindow::YesNoWindow(const String& name, const String& msg)
    {
      m_name = name;
      m_msg  = msg;
    }

    YesNoWindow::YesNoWindow(const String& name,
                             const String& yesBtnText,
                             const String& noBtnText,
                             const String& msg,
                             bool showCancel)
    {
      m_name       = name;
      m_yesText    = yesBtnText;
      m_noText     = noBtnText;
      m_msg        = msg;
      m_showCancel = showCancel;
    }

    void YesNoWindow::Show()
    {
      if (!m_visible)
      {
        return;
      }

      ImGuiIO& io = ImGui::GetIO();
      ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                              ImGuiCond_Once,
                              ImVec2(0.5f, 0.5f));

      ImGui::OpenPopup(m_name.c_str());
      if (ImGui::BeginPopupModal(m_name.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
      {
        if (!m_msg.empty())
        {
          ImGui::Text("%s", m_msg.c_str());
        }

        // Center buttons.
        ImGui::BeginTable("##FilterZoom", m_showCancel ? 5 : 4, ImGuiTableFlags_SizingFixedFit);

        ImGui::TableSetupColumn("##spaceL", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##yes");
        ImGui::TableSetupColumn("##no");
        if (m_showCancel)
        {
          ImGui::TableSetupColumn("##cancel");
        }
        ImGui::TableSetupColumn("##spaceR", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();

        if (ImGui::Button(m_yesText.empty() ? "Yes" : m_yesText.c_str(), ImVec2(120, 0)))
        {
          m_visible = false;
          m_yesCallback();
          ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::TableNextColumn();

        if (ImGui::Button(m_noText.empty() ? "No" : m_noText.c_str(), ImVec2(120, 0)))
        {
          m_visible = false;
          m_noCallback();
          ImGui::CloseCurrentPopup();
        }

        if (m_showCancel)
        {
          ImGui::TableNextColumn();
          if (ImGui::Button("Cancel", ImVec2(120, 0)))
          {
            m_visible = false;
            ImGui::CloseCurrentPopup();
          }
        }

        ImGui::EndTable();
        ImGui::EndPopup();
      }
    }

    // MultiChoiceWindow
    //////////////////////////////////////////////////////////////////////////

    TKDefineClass(MultiChoiceWindow, Window);

    MultiChoiceWindow::MultiChoiceWindow(const String& name, const String& msg)
    {
      m_name = name;
      m_msg  = msg;
      m_buttons.resize(2);
      m_buttons[0].m_name = "Yes";
      m_buttons[1].m_name = "No";
    }

    MultiChoiceWindow::MultiChoiceWindow(const String& name,
                                         const std::vector<ButtonInfo>& buttons,
                                         const String& msg,
                                         bool showCancel)
    {
      m_name       = name;
      m_buttons    = buttons;
      m_msg        = msg;
      m_showCancel = showCancel;
    }

    void MultiChoiceWindow::Show()
    {
      if (!m_visible)
      {
        return;
      }

      ImGuiIO& io = ImGui::GetIO();
      ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                              ImGuiCond_Once,
                              ImVec2(0.5f, 0.5f));

      ImGui::OpenPopup(m_name.c_str());
      if (ImGui::BeginPopupModal(m_name.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
      {
        if (!m_msg.empty())
        {
          ImGui::Text("%s", m_msg.c_str());
        }

        uint columnCount  = (uint) m_buttons.size() + 2;
        columnCount      += m_showCancel ? 1 : 0;
        // Center buttons.
        ImGui::BeginTable("##FilterZoom", columnCount, ImGuiTableFlags_SizingFixedFit);

        ImGui::TableSetupColumn("##spaceL", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##yes");
        ImGui::TableSetupColumn("##no");
        if (m_showCancel)
        {
          ImGui::TableSetupColumn("##cancel");
        }
        ImGui::TableSetupColumn("##spaceR", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();

        for (ButtonInfo& button : m_buttons)
        {
          if (ImGui::Button(button.m_name.c_str(), ImVec2(120, 0)))
          {
            m_visible = false;
            button.m_callback();
            ImGui::CloseCurrentPopup();
          }
          ImGui::TableNextColumn();
        }

        if (m_showCancel)
        {
          ImGui::TableNextColumn();
          if (ImGui::Button("Cancel", ImVec2(120, 0)))
          {
            m_visible = false;
            ImGui::CloseCurrentPopup();
          }
        }

        ImGui::EndTable();
        ImGui::EndPopup();
      }
    }

  } // namespace Editor
} // namespace ToolKit