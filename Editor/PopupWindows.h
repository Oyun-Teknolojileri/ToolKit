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

#pragma once

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

      Type GetType() const override { return Window::Type::InputPopup; }

     public:
      std::function<void(const String& val)> m_taskFn;
      String m_inputVal;
      String m_inputLabel;
      String m_hint;
      ByteArray m_illegalChars;

     private:
      int FilterChars(ImGuiInputTextCallbackData* data);
      bool m_showCancel;
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

      Type GetType() const override { return Window::Type::InputPopup; }

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
      MultiChoiceWindow(const String& name, const std::vector<ButtonInfo>& buttons, const String& msg, bool showCancel);
      void Show() override;

      Type GetType() const override { return Window::Type::InputPopup; }

     public:
      std::vector<ButtonInfo> m_buttons;
      String m_msg;
      bool m_showCancel = false;
    };

  } // namespace Editor
} // namespace ToolKit