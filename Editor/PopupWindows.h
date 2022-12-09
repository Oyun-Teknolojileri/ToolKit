#pragma once

#include "ImGui/imgui.h"
#include "Serialize.h"
#include "Types.h"
#include "UI.h"

namespace ToolKit
{
  namespace Editor
  {
    class StringInputWindow : public Window
    {
     public:
      StringInputWindow(const String& name, bool showCancel);
      void Show() override;
      Type GetType() const override
      {
        return Window::Type::InputPopup;
      }
      std::vector<char> GetIllegalChars()
      {
        return m_illegalChars;
      }

     public:
      std::function<void(const String& val)> m_taskFn;
      String m_inputVal;
      String m_inputLabel;
      String m_hint;

     private:
      bool m_showCancel;
      const std::vector<char> m_illegalChars = {
          '/', ':', '*', '?', '"', '<', '>', '|', '\\'};
    };

    class YesNoWindow : public Window
    {
     public:
      explicit YesNoWindow(const String& name, const String& msg = "");
      YesNoWindow(const String& name,
                  const String& yesBtnText,
                  const String& noBtnText,
                  const String& msg,
                  bool showCancel);
      void Show() override;
      Type GetType() const override
      {
        return Window::Type::InputPopup;
      }

     public:
      std::function<void()> m_yesCallback;
      std::function<void()> m_noCallback;
      String m_msg;
      String m_yesText;
      String m_noText;
      bool m_showCancel = false;
    };

    class MultiChoiceWindow : public Window
    {
     public:
      struct ButtonInfo
      {
        String m_name;
        std::function<void()> m_callback;
      };
      explicit MultiChoiceWindow(const String& name, const String& msg = "");
      MultiChoiceWindow(const String& name,
                        const std::vector<ButtonInfo>& buttons,
                        const String& msg,
                        bool showCancel);
      void Show() override;
      Type GetType() const override
      {
        return Window::Type::InputPopup;
      }

     public:
      std::vector<ButtonInfo> m_buttons;
      String m_msg;
      bool m_showCancel = false;
    };
  } // namespace Editor
} // namespace ToolKit