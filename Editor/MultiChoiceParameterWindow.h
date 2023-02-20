#pragma once
#include "ParameterBlock.h"
#include "UI.h"

namespace ToolKit
{
  namespace Editor
  {
    class MultiChoiceParameterWindow : public TempWindow
    {
     public:
      void OpenCreateWindow(ParameterBlock* parameter);
      void Show() override;
      static MultiChoiceParameterWindow* Instance();
     private:
      bool IsVariantValid();
      void ShowVariant();

     private:
      MultiChoiceVariant m_variant;
      ParameterBlock* m_parameter;
      bool m_menuOpen = false;
    };
  } // namespace Editor
} // namespace ToolKit
