/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once
#include "ParameterBlock.h"
#include "UI.h"

namespace ToolKit
{
  namespace Editor
  {

    class MultiChoiceCraeteWindow : public Window
    {
     public:
      TKDeclareClass(MultiChoiceCraeteWindow, Window);

      void OpenCreateWindow(ParameterBlock* parameter);
      void Show() override;

     private:
      bool IsVariantValid();
      void ShowVariant();

     private:
      MultiChoiceVariant m_variant;
      ParameterBlock* m_parameter;
      bool m_menuOpen = false;
    };

    typedef std::shared_ptr<MultiChoiceCraeteWindow> MultiChoiceCraeteWindowPtr;

  } // namespace Editor
} // namespace ToolKit