/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "UI.h"

namespace ToolKit
{
  namespace Editor
  {

    // StringInputWindow
    //////////////////////////////////////////

    class StringInputWindow : public Window
    {
     public:
      TKDeclareClass(StringInputWindow, Window);

      StringInputWindow();
      virtual void NativeConstruct(const String& name, bool showCancel);
      void Show() override;

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

    typedef std::shared_ptr<StringInputWindow> StringInputWindowPtr;

    // YesNoWindow
    //////////////////////////////////////////

    class YesNoWindow : public Window
    {
     public:
      TKDeclareClass(YesNoWindow, Window);

      YesNoWindow();
      virtual void NativeConstruct(const String& name, const String& msg = "");
      virtual void NativeConstruct(const String& name,
                                   const String& yesBtnText,
                                   const String& noBtnText,
                                   const String& msg,
                                   bool showCancel);
      void Show() override;

     public:
      std::function<void()> m_yesCallback;
      std::function<void()> m_noCallback;
      String m_msg;
      String m_yesText;
      String m_noText;
      bool m_showCancel = false;
    };

    typedef std::shared_ptr<YesNoWindow> YesNoWindowPtr;

    // MultiChoiceWindow
    //////////////////////////////////////////

    struct MultiChoiceButtonInfo
    {
      String m_name;
      std::function<void()> m_callback;
    };

    typedef std::vector<MultiChoiceButtonInfo> MultiChoiceButtonArray;

    class MultiChoiceWindow : public Window
    {
     public:
      TKDeclareClass(MultiChoiceWindow, Window);

      MultiChoiceWindow();

      virtual void NativeConstruct(const String& name, const String& msg = "");

      virtual void NativeConstruct(const String& name,
                                   const MultiChoiceButtonArray& buttons,
                                   const String& msg,
                                   bool showCancel);
      void Show() override;

     public:
      String m_msg;
      bool m_showCancel = false;
      MultiChoiceButtonArray m_buttons;
    };

    typedef std::shared_ptr<MultiChoiceWindow> MultiChoiceWindowPtr;

  } // namespace Editor
} // namespace ToolKit